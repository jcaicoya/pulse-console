# PulseConsole — Operator Runbook

Operational guide for running PulseConsole during the **Bajo Ataque** live show.

---

## Show Scripts

| Script | File | Purpose |
|---|---|---|
| Apertura | `scripts/apertura.yaml` | Opening — simulates a cyber attack, ends with "BAJO ATAQUE" |
| Cierre | `scripts/cierre.yaml` | Closing — cleanup sequence, ends with "Gracias" |

---

## Launching from the Orchestrator

The orchestrator launches PulseConsole automatically when you activate an apertura or cierre
scene. No manual steps needed — the orchestrator passes the correct `--file` and `--screen`
flags and monitors the app status.

---

## Launching Manually

From the package root directory (where `PulseConsole.exe` lives):

```powershell
# Apertura on secondary screen (index 1), fullscreen
.\PulseConsole.exe --file scripts\apertura.yaml --screen 1 --fullscreen

# Cierre on secondary screen (index 1), fullscreen
.\PulseConsole.exe --file scripts\cierre.yaml --screen 1 --fullscreen

# Test on primary screen, windowed
.\PulseConsole.exe --file scripts\apertura.yaml --windowed
```

---

## Display Flags

| Flag | Effect |
|---|---|
| `--fullscreen` | Frameless fullscreen on target screen |
| `--windowed` | Resizable window (for testing) |
| *(none)* | Maximized window |
| `--screen 0` | Primary screen |
| `--screen 1` | Secondary screen / projector |

---

## During the Show

- The app runs autonomously — text appears at scripted timing.
- `pause_until_key` steps stop and wait; press the configured key to continue.
- The mouse cursor is hidden. Move the mouse to confirm the window has focus if needed.
- **Do not press Esc or Q** — they are intentionally disabled to prevent accidental close.
- To close: **Alt+F4**.

---

## If Something Goes Wrong

### App shows an error message on screen

The script file failed to load or parse. Read the error on screen, then:
1. Check the `--file` path is correct and the file exists.
2. Open the YAML file and look for syntax errors (bad indentation, missing quotes, invalid color).
3. Fix the script and relaunch.

### App opens on the wrong screen

The `--screen` index is wrong. Check:
```powershell
# List screens and indices (run in orchestrator or any Qt app)
# Screen 0 = primary, Screen 1 = secondary (projector)
```
Pass the correct `--screen <n>` flag.

### App window is behind other windows

Click the taskbar button, or use `Alt+Tab` to bring it forward. For fullscreen mode, the app
is always-on-top so this should not happen unless another always-on-top window is competing.

### App is frozen / not advancing

A `pause_until_key` step is waiting for input. Click the app window to give it focus, then
press the expected key (e.g., `Space`). If the script uses `pause_until_key: { key: "Any" }`,
press any key.

### Script ends and screen goes black

This is normal — the app stays open showing the last frame after the script finishes.
Close with **Alt+F4** when done.

---

## Editing Show Scripts

Scripts are plain YAML files in `scripts/`. Edit with any text editor.

### Quick reference

```yaml
defaults:
  style: { color: "#00ff00", size: 22, alignH: left, alignV: top }
  type:  { charDelay: 0.03 }   # seconds between characters

steps:
  - write: "Line of text"        # typewriter output
  - wait: 1.5                    # pause in seconds
  - clear: {}                    # clear screen
  - set_style:
      color: "#ff4444"
      size: 64
      alignH: center
      alignV: middle
  - pause_until_key: { key: "Space" }
```

### Color values

Use `#RRGGBB` hex format. Common show colors:

| Color | Hex | Meaning |
|---|---|---|
| Green | `#00ff00` | Normal / system output |
| Amber | `#ffaa00` | Warning |
| Red | `#ff4444` | Alert / attack |
| White | `#ffffff` | End / neutral |

### Tips

- `write` supports `\n` for line breaks within a single step.
- `set_style` only changes the fields you specify — others keep their current value.
- Test scripts with `--windowed` before using them live.
- Keep scripts short — long sequences are hard to interrupt if something goes wrong.

---

## Adding a New Script

1. Create a new `.yaml` file in `scripts/`.
2. Test it: `.\PulseConsole.exe --file scripts\new_script.yaml --windowed`
3. If used by the orchestrator, add it to `config/apps.json` with the `--file` argument.

---

## Build (for developers)

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Output: build\Release\PulseConsole.exe
```

After building, copy the new exe into the package and run `windeployqt` if Qt DLLs changed.
