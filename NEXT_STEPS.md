# NEXT_STEPS — PulseConsole

Ordered by priority. Mark steps as done with `[x]` and commit at least once per step.

## Pending

- [ ] **Persistent service mode — Scene integration** *(see analysis below)*
- [ ] **Loop step** — repeat a block of steps N times or indefinitely (useful for idle loops).
- [ ] **Sound support** — `play_sound: { file: "..." }` step using `QMediaPlayer`.
- [ ] **Script include** — `include: other.yaml` to compose scripts from reusable fragments.

## Done

- [x] **Graceful exit via orchestrator** — `QProcess::terminate()` handled via `closeEvent`;
  engine stopped and `CYBERSHOW_STATUS FINISHED` emitted via `QApplication::aboutToQuit`.
- [x] **Blinking cursor** — script-controlled via `show_cursor: true/false`; off by default.

---

## Analysis: Persistent service mode — Scene integration

### Context

During the show, each Scene follows a fixed sequence:

```
video intro → console message → app (operator-controlled) → console outro → next Scene
```

Rather than launching a new PulseConsole process for each console step (visual gap, process
overhead), PulseConsole runs **once for the whole show** as a persistent rendering slave.
The Orchestrator sends it commands via stdin and reads responses from stdout as usual.

### PulseConsole's role

PulseConsole is a **display surface**, not a scene actor. It stays alive and hidden between
uses. The Orchestrator tells it what to show and when to show it.

### New stdin commands PulseConsole must support

All commands arrive one per line on stdin.

| Command | Behaviour |
|---|---|
| `RUN_SCRIPT <absolute_path>` | Reset state, load script from path, show window, run. Emits `CYBERSHOW_STATUS RUNNING` then `CYBERSHOW_STATUS FINISHED` when done. |
| `FOREGROUND` | `showFullScreen()` + `raise()` + `activateWindow()`. Emits `CYBERSHOW_STATUS FOREGROUND`. |
| `BACKGROUND` | `hide()`. Emits `CYBERSHOW_STATUS BACKGROUND`. |

`RUN_SCRIPT` implies FOREGROUND — no need to send both.

### New stdout responses

```
CYBERSHOW_STATUS FOREGROUND
CYBERSHOW_STATUS BACKGROUND
CYBERSHOW_STATUS RUNNING      # already exists
CYBERSHOW_STATUS FINISHED     # already exists
```

### State reset between scripts

When `RUN_SCRIPT` is received mid-run or after a previous script finished:
- Stop engine
- Clear console buffer
- Reset style to defaults
- Load and start new script

Engine already has `stop()`; `ConsoleWidget` has `clearBuffer()` and `setStyle()`.
A new `Engine::reset()` or reusing `Engine::start()` directly should suffice.

### Implementation sketch

`main.cpp` — add a `QSocketNotifier` on `stdin` (fd 0) to read lines without blocking:
```cpp
auto* notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, &app);
QObject::connect(notifier, &QSocketNotifier::activated, [&](int) {
    // read line, parse command, dispatch
});
```

Or a `QThread` reading `QTextStream(stdin)` and posting events to the main thread.
`QSocketNotifier` is simpler and stays single-threaded.

### What the Orchestrator must do (cross-reference)

This is documented here for context — the actual implementation is in `orchestrator/`.

- Define `ConsoleMessageActor` (or similar) that holds a QProcess handle to the single
  persistent PulseConsole process.
- `scenes.yaml` (under `orchestrator/show/`) defines each scene as an ordered list of steps:
  ```yaml
  scenes:
    - name: "Permisos"
      steps:
        - type: media     file: assets/media/intro_permisos.mp4
        - type: console   script: assets/console/intro_permisos.yaml
        - type: app       qt: permission_qt   android: permission_android   advance: operator
        - type: console   script: assets/console/outro_permisos.yaml
  ```
- Scene sequencer auto-advances on `FINISHED`; waits for operator NEXT on `advance: operator`.
- Window transitions: Orchestrator sends `BACKGROUND` to current foreground actor, then
  `FOREGROUND` (or `RUN_SCRIPT`) to the next one.

### FOREGROUND/BACKGROUND as a shared protocol

This pattern will be needed by all stage Qt apps, not just PulseConsole. Once implemented
here it becomes the standard — other apps adopt it when they are next touched.
Add to `CyberAppMode.h` documentation once stable.

### Show asset layout (Orchestrator side)

```
orchestrator/
└── show/
    ├── scenes.yaml
    └── assets/
        ├── console/
        │   ├── apertura.yaml
        │   ├── intro_permisos.yaml
        │   ├── outro_permisos.yaml
        │   └── ...
        └── media/
            └── ...
```

PulseConsole receives absolute paths — no assumptions about asset location.

### Open questions before implementing

1. **stdin reading strategy** — `QSocketNotifier` vs background `QThread`?
   Preference: `QSocketNotifier` (simpler, no thread sync).
2. **Startup mode** — does PulseConsole start hidden (waiting for first `RUN_SCRIPT`),
   or does it still run a default script on launch?
   Preference: start hidden, show only on `RUN_SCRIPT` or `FOREGROUND`.
3. **`--configure` mode** — configure mode still makes sense (operator can preview scripts);
   persistent service mode activates only under `--show` / `--live`.
