# DRIVE

DRIVE es un framework modular de simulación para Unreal Engine orientado a escenarios controlados, ejecución determinista y separación clara entre lógica de simulación, estado y representación en mundo.

## Objetivo

Permitir integrar desde Unreal Engine una simulación funcional en pocos pasos, con un flujo de inicialización reproducible y una arquitectura preparada para crecer mediante Systems, escenarios y componentes de integración.

## Contenido de la documentación

- [01_Instalacion.md](Docs/01_Instalacion.md)  
  Guía de instalación e integración del plugin en un proyecto Unreal.

- [02_Piezas_Disponibles.md](Docs/02_Piezas_Disponibles.md)  
  Resumen de las piezas principales del framework y su responsabilidad.

- [03_Flujo_Recomendado.md](Docs/03_Flujo_Recomendado.md)  
  Flujo recomendado de uso para arrancar una simulación correctamente.

- [04_Ejemplo_Minimo_Funcional.md](Docs/04_Ejemplo_Minimo_Funcional.md)  
  Ejemplo completo paso a paso basado en la integración actual del plugin.

- [05_Troubleshooting.md](Docs/05_Troubleshooting.md)  
  Problemas frecuentes y comprobaciones rápidas.

## Flujo rápido

1. Copiar o incluir el plugin DRIVE en `Plugins/DRIVE`.
2. Compilar el proyecto.
3. Configurar `UDriveGlobalSettings` con un `UDriveGlobalConfigAsset`.
4. Usar `UDriveGameInstance` o una derivada.
5. Ejecutar el proyecto en PIE o Game.
6. Verificar en logs que el Host, el WorldSubsystem y el Runtime se inicializan correctamente.

## Estado actual del ejemplo documentado

El ejemplo mínimo funcional documentado en este entregable se basa en:
- arranque automático desde `UDriveGameInstance`,
- creación del `UWorldSimulationSubsystem`,
- inicialización del `FDriveSimulationHost`,
- construcción del runtime,
- registro de systems activos,
- y avance de simulación mediante ticker.

Este flujo permite validar una integración funcional de DRIVE aunque el nivel de demostración visual final no forme parte todavía del entregable.