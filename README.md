# PulseConsole

Motor de presentación de consola a pantalla completa para **Bajo Ataque** y shows similares. Ejecuta secuencias guionizadas desde YAML con efecto de máquina de escribir, pausas temporizadas, cambios de estilo y estética CRT.

## Qué es

PulseConsole es una superficie escénica para mostrar mensajes de consola en proyector o pantalla grande. Puede funcionar en solitario o ser lanzado por el orchestrator.

## Funcionalidad principal

- scripting en YAML
- efecto typewriter
- pausas temporizadas
- limpieza de pantalla
- cambios dinámicos de estilo
- pausa hasta tecla
- efectos CRT: scanlines, glow, vignette y flicker

## Arquitectura

### Flujo de datos

```text
YAML / recurso embebido
  -> ScriptLoader
  -> Engine
  -> ConsoleWidget
```

### Componentes

- `ScriptLoader`: parsea YAML a una estructura de script
- `Engine`: máquina de estados de ejecución
- `ConsoleWidget`: render de texto y efectos CRT
- `CyberAppMode`: integración de flags comunes con el ecosistema Cybershow

## Pasos del script

Los scripts usan estos tipos de paso:

| Paso | Descripción |
|---|---|
| `write` | escribe texto |
| `wait` | pausa temporizada |
| `clear` | limpia pantalla |
| `set_style` | cambia color, tamaño y alineación |
| `pause_until_key` | espera una tecla |
| `show_cursor` | muestra u oculta cursor |

## Recursos y estructura

```text
pulse_console/
├── .claude/CLAUDE.md
├── CODEX.md
├── README.md
├── RUNBOOK.md
├── NEXT_STEPS.md
├── resources/
├── scripts/
└── src/
```

## Integración con Cybershow

- soporta `--live` y `--demo`
- normaliza también `--show` y `--design`
- emite líneas `CYBERSHOW_STATUS` por stdout
- puede ser controlado por orchestrator

## Tecnología

| Capa | Tecnología |
|---|---|
| Plataforma | Windows |
| Framework | Qt Widgets 6.7.3 |
| Lenguaje | C++23 |
| Parser YAML | yaml-cpp |
| Build | CMake |

## Estado actual

- ejecución de scripts YAML funcional
- soporte multi-pantalla
- fullscreen/windowed/maximized
- integración básica con orchestrator
- seguridad escénica para evitar cierres accidentales
