# PulseConsole — AI Assistant Context

> Working context for Claude, Codex, Gemini, or any coding assistant.
> Keep this file updated as the project evolves.

---

## 1. Project Summary

**PulseConsole** is a fullscreen console presentation engine for the live cybersecurity show
**Bajo Ataque**. It renders YAML-scripted sequences on a projector screen: typewriter text,
timed pauses, style changes, and CRT visual effects.

Built in **C++23 / Qt 6.7.3 (Widgets) / yaml-cpp / CMake / MSVC** for Windows.

It runs as a standalone app or launched by the **Cybershow Orchestrator**, which controls it
via command-line flags and reads `CYBERSHOW_STATUS` lines from its stdout.

---

## 2. Repository Layout

```
pulse_console/
├── CMakeLists.txt
├── README.md
├── CLAUDE.md                    # This file
├── RUNBOOK.md                   # Operator guide
├── scripts/                     # Runtime show scripts
│   ├── apertura.yaml            # Opening — "BAJO ATAQUE" sequence
│   └── cierre.yaml              # Closing — "Gracias" sequence
├── resources/
│   ├── pulseconsole.qrc
│   └── scripts/
│       ├── default.yaml         # Embedded fallback (compiled into exe)
│       └── second_screen.yaml
└── src/
    ├── main.cpp
    ├── CyberAppMode.h/cpp       # Shared orchestrator integration (parseAppLaunchOptions)
    ├── engine/
    │   ├── Steps.h              # Script data structures
    │   ├── ScriptLoader.h/cpp   # YAML → Steps
    │   └── Engine.h/cpp        # Execution state machine
    └── ui/
        └── ConsoleWidget.h/cpp  # Rendering widget
```

---

## 3. Architecture

### 3.1 Data flow

```
YAML file / resource
      │
  ScriptLoader         parses YAML → pc::Script (vector of Steps)
      │
   Engine              state machine: drives steps one at a time via QTimer
      │
 ConsoleWidget         Qt widget: renders text, emits keyPressed(int)
```

### 3.2 Steps (Steps.h)

All step types are held in a `std::variant<WriteStep, WaitStep, ClearStep, SetStyleStep, PauseUntilKeyStep>`.

| Type | Key fields |
|---|---|
| `WriteStep` | `text: QString` |
| `WaitStep` | `seconds: double` |
| `ClearStep` | *(empty)* |
| `SetStyleStep` | `style: Style` (partial — only set fields override) |
| `PauseUntilKeyStep` | `qtKey: int` (0 = any key) |

`Style` holds `color (QColor)`, `sizePx (int)`, `alignH`, `alignV`.
`ScriptDefaults` wraps a `Style` + `TypeParams { charDelaySeconds }`.
`Script` = `{ defaults: ScriptDefaults, steps: vector<Step> }`.

### 3.3 Engine (Engine.h/cpp)

Single-threaded, event-driven. `QTimer` fires every tick; `onTick()` advances state.

```
enum class Mode { Idle, Writing, Waiting, Paused }
```

- **Writing**: emits one character per tick at `charDelaySeconds` interval.
- **Waiting**: counts down `waitRemainingMs`.
- **Paused**: does nothing until `onKeyPressed(int)` fires via ConsoleWidget signal.
- After each step completes: calls `startNextStep()`. When all steps are done: stays Idle.

Key lookup for `pause_until_key`: `"Space"` → `Qt::Key_Space`, `"Any"` → 0, single char → `Qt::Key_A` etc.

### 3.4 ConsoleWidget (ui/ConsoleWidget.h/cpp)

Custom `QWidget`. Manages a line buffer (max 2000 lines).

Public interface:
```cpp
void appendText(const QString& text);   // add chars, handle \n
void appendLine(const QString& line);   // add full line + newline
void setText(const QString& text);      // replace all content
void setStyle(QColor color, int sizePx, AlignH, AlignV);
void clearScreen();
```

Signal: `keyPressed(int qtKey)` — emitted on any `keyPressEvent`.

**CRT effects** (always on, rendered in `paintEvent`):
- Scanlines: horizontal lines every 3 px at 20% opacity
- Vignette: radial gradient darkening at edges (cached pixmap, rebuilt on resize)
- Glow: text drawn 4 times offset + original for soft bloom
- Flicker: random `QPainter::setOpacity()` variation (strength ~0.0–0.2)

Font: Consolas, monospace.

### 3.5 Cybershow Integration (CyberAppMode.h/cpp)

Shared header with the orchestrator ecosystem. `parseAppLaunchOptions(QStringList)` parses:

| Flag | Field |
|---|---|
| `--live` | `options.live = true` |
| `--demo` | `options.live = false` |
| `--fullscreen` | `options.fullscreen = true` |
| `--windowed` | `options.windowed = true` |
| `--screen <n>` | `options.screenIndex = n` |

`main.cpp` normalizes `--show` → `--live` and `--design` → `--demo` before calling this.

Stdout protocol (read by orchestrator):
```
CYBERSHOW_STATUS READY      # emitted after options parsed, before script loads
CYBERSHOW_STATUS RUNNING    # emitted after engine->start()
```

### 3.6 Script loading

`--file <path>` loads from disk. No flag → embedded `:/scripts/default.yaml`.
Errors are displayed inside the console widget itself (not a dialog), so the stage shows
a readable error message if a script fails to parse.

---

## 4. Script Format

```yaml
defaults:
  style: { color: "#00ff00", size: 22, alignH: left, alignV: top }
  type:  { charDelay: 0.03 }

steps:
  - write: "text here"
  - wait: 1.5
  - clear: {}
  - set_style: { color: "#ff4444", size: 64, alignH: center, alignV: middle }
  - pause_until_key: { key: "Space" }
```

All `set_style` fields are optional — only specified fields change.
`pause_until_key` key values: `"Space"`, `"Enter"`, `"Return"`, `"Any"`, or single char.

---

## 5. Build

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Requires Qt 6.7.3, MSVC 2022, CMake. `yaml-cpp` is fetched automatically via `FetchContent`.
No FFmpeg or multimedia modules needed — this app is Widgets only.

---

## 6. Known Gotchas

| Issue | Solution |
|---|---|
| `--screen` with invalid index | Falls back to primary screen silently |
| Script on wrong screen | Check orchestrator's `setStageGeometry` / `scheduleWindowMove` config |
| CRT flicker too strong | Adjust flicker strength constant in `ConsoleWidget.cpp` |
| yaml-cpp fetch fails on air-gapped machine | Pre-vendor yaml-cpp or add it via vcpkg |
| MSVC: `auto` range-for over `QMap` fails | Use explicit types in range-for |

---

## 7. What Is Built and Working

- YAML script parsing with descriptive error messages
- Full step set: write, wait, clear, set_style, pause_until_key
- Typewriter effect with per-script configurable delay
- Fullscreen / windowed / maximized display modes
- Multi-screen targeting via `--screen <n>`
- CRT effects: scanlines, vignette, glow, flicker
- Cybershow orchestrator protocol (`CYBERSHOW_STATUS`, `--live`/`--demo`, `--show`/`--design` normalization)
- Stage safety: no accidental close, hidden cursor, always-on-top

---

## 8. Next Steps

1. **Blinking cursor** — a blinking block or underscore after the last character.
2. **Loop step** — repeat a block of steps N times or indefinitely (useful for idle loops).
3. **Sound support** — `play_sound: { file: "..." }` step using `QMediaPlayer`.
4. **Script include** — `include: other.yaml` to compose scripts from reusable fragments.
5. **Graceful exit via orchestrator** — respond to a signal or named pipe to exit cleanly
   without requiring Alt+F4.

---

## 9. Design Rules (Non-Negotiable)

1. All paths relative to the executable — never hardcoded absolute paths.
2. Script errors must be visible on the stage (render in ConsoleWidget), never silent.
3. No blocking calls — all execution is timer-driven.
4. Stage safety: `Esc` / `Q` must not close the app. Only `Alt+F4`.
5. CRT effects are always on — they are part of the show aesthetic.
6. Code, identifiers, comments in English. Script content in Spanish (show language).
7. The app must start and show something immediately — never a blank black screen with no output.
