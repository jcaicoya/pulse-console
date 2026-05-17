# PulseConsole — Operator Runbook

## Scripts del show

| Script | Archivo | Propósito |
|---|---|---|
| Apertura | `scripts/apertura.yaml` | secuencia inicial |
| Cierre | `scripts/cierre.yaml` | secuencia final |

## Arranque desde Orchestrator

El orchestrator puede lanzar PulseConsole automáticamente con los flags y el script correctos.

## Arranque manual

Desde la raíz del paquete:

```powershell
.\PulseConsole.exe --file scripts\apertura.yaml --screen 1 --fullscreen
.\PulseConsole.exe --file scripts\cierre.yaml --screen 1 --fullscreen
.\PulseConsole.exe --file scripts\apertura.yaml --windowed
```

## Flags de pantalla

| Flag | Efecto |
|---|---|
| `--fullscreen` | fullscreen sin bordes |
| `--windowed` | ventana redimensionable |
| sin flag | ventana maximizada |
| `--screen 0` | pantalla principal |
| `--screen 1` | pantalla secundaria / proyector |

## Durante el show

- la app ejecuta la secuencia de forma autónoma
- si hay un `pause_until_key`, hay que pulsar la tecla indicada
- el cursor está oculto
- `Esc` y `Q` no cierran la app
- para cerrar: `Alt+F4`

## Si algo falla

### Error visible en pantalla

1. comprobar ruta del script
2. comprobar sintaxis YAML
3. corregir y relanzar

### La app aparece en la pantalla equivocada

Revisar el índice de `--screen`.

### La app parece congelada

Probablemente está esperando un `pause_until_key`. Dar foco a la ventana y pulsar la tecla esperada.

### El script termina y la pantalla queda fija

Es normal. La app permanece abierta mostrando el último frame hasta cerrar con `Alt+F4`.

## Edición de scripts

Los scripts están en `scripts/` y usan YAML.

Ejemplo mínimo:

```yaml
defaults:
  style: { color: "#00ff00", size: 22, alignH: left, alignV: top }
  type:  { charDelay: 0.03 }

steps:
  - write: "Line of text"
  - wait: 1.5
  - clear: {}
  - set_style:
      color: "#ff4444"
      size: 64
      alignH: center
      alignV: middle
  - pause_until_key: { key: "Space" }
```

### Colores habituales

| Color | Hex | Uso |
|---|---|---|
| Verde | `#00ff00` | salida normal |
| Ámbar | `#ffaa00` | aviso |
| Rojo | `#ff4444` | alerta |
| Blanco | `#ffffff` | cierre o neutro |

## Build y release

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Si hace falta generar paquete de distribución, usar el flujo del proyecto correspondiente.

## Consideraciones operativas

- solo `Alt+F4` debe cerrar la app
- los errores de script deben verse en pantalla
- la app no debe quedarse en negro sin feedback
