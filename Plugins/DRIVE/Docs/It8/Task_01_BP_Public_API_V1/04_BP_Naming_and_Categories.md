# 04 - Naming & Categories

## Prefijos y naming
- Clases BP-facing: `UDrive*` / `ADrive*`
- Subsystem: se mantiene `UWorldSimulationSubsystem`, pero las funciones expuestas usarán prefijo `Drive` en DisplayName si hace falta.

## Categorías Blueprint (UFUNCTION/UPROPERTY Category)
Se congelan estas categorías:

- `DRIVE|Simulation`
- `DRIVE|Scenario`
- `DRIVE|Lifecycle`
- `DRIVE|Events`
- `DRIVE|Debug`

## Defaults v1
- AutoStart (si existe actor controlador): **false**
- FixedDtSeconds default: **0.0166667** (60 Hz) 
- MaxSubstepsPerFrame: valor seguro (ej: 8)
- Seed default: 0 