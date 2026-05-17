# NEXT_STEPS — PulseConsole

## Pendiente

- [ ] Modo servicio persistente para integración por escenas con Orchestrator.
- [ ] Añadir paso `loop` para repetir bloques.
- [ ] Añadir soporte de sonido con `play_sound`.
- [ ] Añadir `include` de scripts YAML reutilizables.

## Hecho

- [x] Salida controlada mediante orchestrator con cierre limpio.
- [x] Cursor parpadeante controlado por script.

## Nota de diseño

La integración como servicio persistente implica:

- comandos por stdin como `RUN_SCRIPT`, `FOREGROUND` y `BACKGROUND`
- respuestas `CYBERSHOW_STATUS FOREGROUND` y `CYBERSHOW_STATUS BACKGROUND`
- reset completo del estado entre scripts
