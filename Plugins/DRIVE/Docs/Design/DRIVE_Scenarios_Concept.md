# DRIVE — Concepto de Escenario

## Definición
Un **Escenario** en DRIVE es una **descripción declarativa (data-only)** de una simulación reproducible.
Su propósito es describir “qué se quiere simular” sin ejecutar la simulación ni depender del runtime.

## Objetivos
- Ser una **entrada** clara y desacoplada del runtime.
- Permitir **reproducibilidad**: el mismo escenario debe poder generar el mismo inicio de simulación.
- Separar **datos** (escenario) de **ejecución** (runtime/systems).

## Responsabilidades
- Contener la información necesaria para **describir** una simulación:
  - Identidad del escenario (id/nombre).
  - Selección/configuración conceptual de qué se simula.
  - Parámetros iniciales (conceptualmente).
  - Referencias a elementos de la simulación usando **identificadores estables**, no punteros del runtime.
- Ser **serializable/validable/versionable** de forma natural.

## No-responsabilidades
- No ejecuta nada.
- No contiene estado mutable de ejecución.
- No mantiene punteros a systems, subsystems, UWorld, actores, componentes runtime.
- No depende de orden de inicialización del runtime.
- No resuelve cómo se traduce a estado de simulación.

## Contenido mínimo
Un escenario debería poder expresar, como mínimo, estas categorías:
- **Identidad**: ScenarioId.
- **Configuración global referenciada**: referencia lógica a presets/config.
- **Parámetros deterministas**: valores iniciales.
- **Selección de sistemas o features**: qué systems están activos o qué modo se usa.
- **Referencias a “cosas” del mundo**: entidades iniciales, spawns, plantillas, etc., siempre por id lógico.

## Principios de diseño
- **Data-only**: el escenario es un contenedor de datos, no de lógica.
- **Idempotencia**: describir el escenario no provoca efectos colaterales.
- **Desacoplo**: el runtime debe poder existir sin escenario, y el escenario sin runtime.
- **Determinismo**: el escenario debe ser suficiente para reconstruir el mismo arranque.
