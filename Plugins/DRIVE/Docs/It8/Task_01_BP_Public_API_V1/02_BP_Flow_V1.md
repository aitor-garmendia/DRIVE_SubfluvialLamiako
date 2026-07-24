# 02 - BP Flow v1

## Estado de alto nivel
Se define una máquina de estados visible a BP:

- **Uninitialized**: no hay runtime listo / no hay host activo
- **Ready**: host listo pero sin running
- **Running**: simulación en ejecución
- **Stopping**: transición de parada
- **Error**: fallo no recuperable sin Stop/Reset


## Flujo oficial
1. **Obtener Subsystem**
   - BP: GetWorldSimulationSubsystem(WorldContext)

2. **LoadScenario**
   - Carga/valida scenario desde asset/config
   - Si falla: State no pasa a Ready, devuelve false + log + OnError

3. **Start**
   - Solo permitido si State == Ready
   - Si ok: pasa a Running y dispara OnStarted

4. **Running**
   - El avance ocurre por el Host/Subsystem Tick
   - BP puede suscribirse a eventos

5. **Stop**
   - Permitido si Running o Ready (idempotente)
   - Si ok: pasa a Ready (o Uninitialized si se decide reset total) y dispara OnStopped

## Reglas de orden (hard rules)
- No se permite Start sin LoadScenario exitoso.
- LoadScenario no se permite si Running.
- Stop siempre debe ser seguro.