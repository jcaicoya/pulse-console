# PulseConsole — AI Assistant Context

Instrucciones de trabajo específicas para este subproyecto.

## Lectura obligatoria al empezar

Antes de trabajar aquí, lee y aplica también:

- `README.md`
- `RUNBOOK.md`
- `NEXT_STEPS.md`

## Qué contiene cada archivo

- `README.md`: qué es PulseConsole, arquitectura, pasos de script e integración.
- `RUNBOOK.md`: operación, arranque, uso en show y edición básica de scripts.
- `NEXT_STEPS.md`: pendientes y notas de diseño inmediatas.
- `.claude/CLAUDE.md`: reglas de trabajo específicas de este directorio.

No dupliques información entre estos archivos. Cada dato debe vivir en un único sitio.

## Forma de trabajar en este directorio

- El usuario se encarga de compilar, probar, empaquetar, hacer commits y hacer push.
- Si cambias formato de script, integración con orchestrator, contrato de stdout o backlog, actualiza el archivo correspondiente.
- Tras cada commit, `README.md`, `RUNBOOK.md` y `NEXT_STEPS.md` deben seguir reflejando el estado real del proyecto.
- Hay que preservar las reglas de seguridad escénica: solo `Alt+F4` cierra, y los errores deben ser visibles en pantalla.

## Reglas importantes de esta app

- No introducir llamadas bloqueantes en la ejecución del script.
- Mantener la ejecución dirigida por temporizadores/eventos.
- No ocultar silenciosamente errores de script.
- La app debe mostrar algo útil inmediatamente al arrancar.

## Alcance de este archivo

Este archivo no debe repetir documentación general de producto ni instrucciones operativas de uso; eso pertenece a `README.md` o `RUNBOOK.md`.
