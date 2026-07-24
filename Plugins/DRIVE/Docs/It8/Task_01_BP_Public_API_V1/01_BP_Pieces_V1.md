# 01 - BP Pieces v1

## P0. UWorldSimulationSubsystem
**Tipo:** World Subsystem
**Responsabilidad BP:** Puerta principal para controlar el ciclo de simulación desde Unreal (PIE/Play), sin acceder a Core directamente.
**Notas:** Internamente delega en Host/Runtime/Clock.

### Exposición BP
- **Get State** (enum)
- **Load Scenario** (por asset/config)
- **Start**
- **Stop**
- **(Opcional v1)**: Query metrics / snapshot (solo lectura)

---

## P1. UDriveBlueprintLibrary (Utility / Static BP)
**Tipo:** Blueprint Function Library
**Responsabilidad BP:** Helpers estáticos para acceder a DRIVE desde cualquier Blueprint sin referencias manuales:
- Obtener el subsystem de un WorldContext
- Helpers de conversión/formatos/validación (sin lógica de runtime)

---

## P2. ADriveSimulationController (Actor de orquestación)
**Tipo:** Actor
**Responsabilidad BP:** Permitir el flujo “drop-in” en un nivel:
- Referencia a Scenario Asset
- Auto-start opcional
- Exponer eventos a Blueprints (OnStarted/OnStopped/OnError)

---

## P3. UDriveScenarioAsset (Asset BP-friendly)
**Tipo:** DataAsset
**Responsabilidad BP:** Representar un escenario cargable desde editor, con defaults y validación editor.
Contiene:
- Identificador/nombre
- Lista de Systems a activar
- Parámetros iniciales (seed, fixed dt, etc.)
- Lista de spawns iniciales (opcional)

---

## P4. UDriveAgentSpawnerComponent
**Tipo:** ActorComponent
**Responsabilidad BP:** Permitir spawnear entidades/agentes desde actor en el nivel con parámetros editables.