# 03 - BP Events v1

## Eventos del Subsystem
Estos eventos deben estar disponibles para bind desde BP:

- **OnScenarioLoaded(success, message)**
- **OnStarted()**
- **OnStopped()**
- **OnStateChanged(oldState, newState)**
- **OnError(errorCode, message)**

## Eventos de ejecución
- **OnFixedStep(stepIndex, simTimeSeconds)**
- **OnDeterminismSummary(execHash, steps, simTime)**

## Política de payload
- No se exponen structs internos del Core directamente a BP.
- Payloads: tipos BP-friendly (bool, int32, float/double, FString, FName, enums, structs UE simples).