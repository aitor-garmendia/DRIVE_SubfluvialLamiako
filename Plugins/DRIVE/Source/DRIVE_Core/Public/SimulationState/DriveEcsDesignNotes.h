#pragma once

/**
 * DRIVE - ECS Design Notes (Iteración 3, Tarea 1)
 *
 * 1) Entidad:
 *    - Es un identificador estable (FDriveEntityId).
 *    - No contiene datos.
 *    - Se crea/destruye mediante un registro (Tarea 2).
 *    - El par (Index, Generation) evita acceder a entidades destruidas.
 *
 * 2) Componente:
 *    - Es un bloque de datos asociado a una entidad.
 *    - Debe ser modular, desacoplado y extensible.
 *    - No contiene lógica; la lógica vive en Systems.
 *
 * 3) Identificación de componentes:
 *    - Cada tipo de componente se asocia a un TypeId (FDriveComponentTypeId).
 *    - En Tarea 1, TypeId se define como hash del nombre del componente.
 *    - En Tarea 3, se podrá migrar a registro/IDs densos manteniendo APIs.
 *
 * 4) Reglas de arquitectura:
 *    - Runtime (FScenarioRuntime) ejecuta Systems por fixed-step.
 *    - Systems operan sobre subconjuntos del estado consultando componentes (Tarea 4/5).
 *    - El estado de simulación centralizado será la fuente única de verdad (Tarea 5).
 */