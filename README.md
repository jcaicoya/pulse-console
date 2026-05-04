# PulseConsole

Fullscreen, stage-ready console presentation engine for **Bajo Ataque** and similar live shows.
Built with **C++23**, **Qt 6.7.3 (Widgets)**, and **yaml-cpp**.

Executes scripted console sequences from YAML files with typewriter animation, timed pauses,
dynamic style changes, and CRT visual effects. Designed to run on a projector screen under
orchestrator control or standalone.

---

## Features

- YAML-based scripting (external file or embedded resource)
- Typewriter effect with configurable character delay
- Timed waits, screen clear, style changes, pause-until-key
- CRT effects: scanlines, vignette, glow, flicker
- Fullscreen frameless window, always-on-top, hidden cursor
- Multi-screen support via `--screen <index>`
- Cybershow orchestrator integration (`CYBERSHOW_STATUS` protocol, `--live` / `--demo` modes)
- Only closable via **Alt+F4** — stage safety

---

## Project Structure

```
pulse-console/
├── src/
│   ├── main.cpp
│   ├── engine/
│   │   ├── Steps.h           # Script data structures (std::variant step types)
│   │   ├── ScriptLoader.h/cpp
│   │   └── Engine.h/cpp      # Execution state machine (Idle/Writing/Waiting/Paused)
│   └── ui/
│       └── ConsoleWidget.h/cpp  # Qt widget with CRT rendering
├── resources/
│   ├── pulseconsole.qrc
│   └── scripts/
│       ├── default.yaml         # Embedded fallback script
│       └── second_screen.yaml
├── scripts/                     # Show scripts (loaded at runtime)
│   ├── apertura.yaml            # Opening sequence ("BAJO ATAQUE")
│   └── cierre.yaml              # Closing sequence ("Gracias")
├── CMakeLists.txt
└── README.md
```

---

## Running the Application

### Default (embedded script)

```
PulseConsole.exe
```

### External script

```
PulseConsole.exe --file scripts\apertura.yaml
PulseConsole.exe --file=scripts\apertura.yaml
```

### Display options

| Flag | Effect |
|---|---|
| *(none)* | Maximized window |
| `--fullscreen` | Frameless fullscreen |
| `--windowed` | Resizable window |
| `--screen <n>` | Target screen index (0 = primary) |

### Orchestrator launch modes

| Flag | Meaning |
|---|---|
| `--live` | Live show mode |
| `--demo` | Rehearsal/demo mode |

The orchestrator may also pass `--show` or `--design`; these are normalized internally.

---

## Script Format (YAML)

```yaml
defaults:
  style: { color: "#00ff00", size: 22, alignH: left, alignV: top }
  type:  { charDelay: 0.03 }

steps:
  - write: "Iniciando módulos..."
  - wait: 1.2
  - set_style: { color: "#ff4444", size: 64, alignH: center, alignV: middle }
  - write: "BAJO ATAQUE"
  - clear: {}
  - pause_until_key: { key: "Space" }
```

### Step reference

| Step | Fields | Description |
|---|---|---|
| `write` | text (string) | Outputs text with typewriter effect; `\n` for newline |
| `wait` | seconds (float) | Timed pause |
| `clear` | *(empty map)* | Clears the screen |
| `set_style` | `color`, `size`, `alignH`, `alignV` | Changes current style (all optional) |
| `pause_until_key` | `key` | Blocks until the specified key is pressed |

### Alignment

- `alignH`: `left` `center` `right`
- `alignV`: `top` `middle` `bottom`

### Keys for `pause_until_key`

`"Space"` `"Enter"` `"Return"` `"Any"` or a single character (`"A"`, `"q"`, etc.)

---

## Build

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Requirements: Windows 11, MSVC 2022, Qt 6.7.3 (Widgets), CMake. `yaml-cpp` is fetched
automatically via `FetchContent`.

---

## Stage Safety

- Frameless, always-on-top, hidden cursor
- `Esc` and `Q` do **not** close the app
- Only **Alt+F4** closes the application
- Windows system dialogs (UAC, Ctrl+Alt+Del) cannot be suppressed

---

## License

MIT — part of the Cybershow / Bajo Ataque project.
