# PulseConsole

PulseConsole is a fullscreen, stage-ready console presentation engine
built with **C++23**, **Qt 6 (Widgets)**, and **yaml-cpp**.

It executes scripted console sequences defined in YAML, supporting:

-   Typewriter text animation
-   Timed waits
-   Screen clearing
-   Dynamic style changes (color, size, alignment)
-   Pause-until-key interaction
-   Fullscreen, frameless, always-on-top display

Designed for live demos, tech shows, and controlled stage environments.

------------------------------------------------------------------------

## Features

-   YAML-based scripting language
-   Clean separation of responsibilities:
   -   `ScriptLoader` → YAML parsing
   -   `Engine` → Execution logic
   -   `ConsoleWidget` → Rendering
-   Non-blocking execution using `QTimer`
-   Typewriter effect with configurable character delay
-   Pause on specific key (e.g., `"Space"`)
-   Frameless fullscreen window
-   Always-on-top window mode
-   Only closable via **Alt+F4** (stage safety)

------------------------------------------------------------------------

## Project Structure

    src/
     ├── main.cpp
     ├── ui/
     │    ├── ConsoleWidget.h
     │    └── ConsoleWidget.cpp
     └── engine/
          ├── Steps.h
          ├── ScriptLoader.h
          ├── ScriptLoader.cpp
          ├── Engine.h
          └── Engine.cpp

    resources/
     ├── pulseconsole.qrc
     └── scripts/
          └── default.yaml

------------------------------------------------------------------------

## Script Format (YAML)

Example:

``` yaml
defaults:
  style: { color: "#00ff00", size: 20, alignH: left, alignV: top }
  type:  { charDelay: 0.02 }

steps:
  - write: "Hello, World!"
  - wait: 1.0
  - clear: {}
  - write: "Here we go"
  - wait: 1.0
  - clear: {}
  - set_style: { color: "#ff0000", size: 50, alignH: center, alignV: middle }
  - write: "Alert!"
  - pause_until_key: { key: "Space" }
  - clear: {}
  - set_style: { color: "#ffffff", size: 20, alignH: left, alignV: top }
  - write: "Bye"
```

### Supported Steps

Step                Description
  ------------------- ---------------------------------------
`write`             Writes text using typewriter effect
`wait`              Waits given seconds
`clear`             Clears the screen
`set_style`         Changes color, size, alignment
`pause_until_key`   Waits until a specific key is pressed

### Alignment Options

-   `alignH`: `left`, `center`, `right`
-   `alignV`: `top`, `middle`, `bottom`

### Keys for `pause_until_key`

Supported values: - `"Space"` - `"Enter"` - `"Return"` - `"Any"` -
Single characters (`"A"`, `"q"`, etc.)

------------------------------------------------------------------------

## Running the Application

### Default (embedded script)

Runs the embedded resource script:

    PulseConsole.exe

------------------------------------------------------------------------

### Load Script from File

You can load an external YAML script:

    PulseConsole.exe --script path\to\script.yaml

Or:

    PulseConsole.exe --script=path\to\script.yaml

If no `--script` argument is provided, the embedded resource is used.

------------------------------------------------------------------------

## Stage Safety

-   Window is **frameless**
-   Window is **always on top**
-   Mouse cursor is hidden
-   `Esc` and `Q` do NOT close the app
-   Only **Alt+F4** closes the application

Note: Windows system dialogs (UAC, Ctrl+Alt+Del) cannot be overridden.

------------------------------------------------------------------------

## Build Requirements

-   Windows 11
-   MSVC 2022
-   Qt 6.7.3 (Widgets)
-   CMake
-   yaml-cpp (via FetchContent)

CMake must include:

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

And:

    target_link_libraries(PulseConsole
        PRIVATE
            Qt6::Widgets
            yaml-cpp::yaml-cpp
    )

------------------------------------------------------------------------

## Development Notes

-   Line endings normalized to LF via `.gitattributes`
-   Includes configured using:

```{=html}
<!-- -->
```
    target_include_directories(PulseConsole
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )

-   Engine is single-threaded and event-driven
-   No blocking calls
-   Designed for predictable live execution

------------------------------------------------------------------------

## Future Ideas

-   Blinking cursor
-   Neon glow / CRT effect
-   Multiple scenes support
-   Script include/import
-   Loop constructs
-   Timeline visualizer
-   Kiosk mode

------------------------------------------------------------------------

## License

MIT License

------------------------------------------------------------------------

PulseConsole is part of the Cybershow project.
