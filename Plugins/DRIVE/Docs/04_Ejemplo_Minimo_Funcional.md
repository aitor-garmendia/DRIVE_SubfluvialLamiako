# EJEMPLO MÍNIMO FUNCIONAL

## 1. Introducción

Este documento presenta un ejemplo mínimo funcional que permite verificar que el framework DRIVE está correctamente instalado e integrado dentro de un proyecto de Unreal Engine.

El objetivo de este ejemplo no es crear una simulación compleja, sino demostrar que el flujo completo del framework funciona correctamente. Esto incluye la inicialización del sistema, la creación del runtime de simulación, el registro de systems y la ejecución del ciclo de simulación.

Siguiendo los pasos descritos en este documento es posible comprobar que la simulación se inicializa correctamente y que el runtime comienza a ejecutarse dentro del entorno de Unreal Engine.

## 2. Objetivo del ejemplo

El objetivo principal de este ejemplo es validar que los componentes principales de DRIVE funcionan correctamente dentro del proyecto.

En particular se pretende comprobar los siguientes aspectos:

- el plugin DRIVE está correctamente instalado

- el proyecto compila sin errores

- la GameInstance inicializa el framework

- el Simulation Host se crea correctamente

- el World Simulation Subsystem se inicializa

- el runtime de simulación se construye correctamente

- los systems se registran y se inicializan

- la simulación comienza a ejecutarse

Si todos estos pasos se completan correctamente, se puede considerar que el framework está funcionando correctamente dentro del proyecto.

## 3. Preparación del proyecto

Antes de ejecutar este ejemplo se deben haber completado los pasos descritos en los documentos anteriores.

En particular se debe comprobar que:

- el plugin DRIVE está instalado dentro de la carpeta Plugins del proyecto

- el proyecto compila correctamente

- existe un asset de configuración global del framework

- el proyecto utiliza una GameInstance derivada de DriveGameInstance

Una vez cumplidos estos requisitos se puede proceder a ejecutar el ejemplo.

## 4. Crear un mapa de prueba

Para ejecutar el ejemplo mínimo funcional es suficiente con utilizar un mapa simple dentro del proyecto.

Se puede crear un mapa vacío desde el editor de Unreal o utilizar uno existente.

El mapa no necesita contener actores específicos para la simulación, ya que el objetivo de este ejemplo es comprobar el proceso de inicialización del framework.

Una vez creado o seleccionado el mapa, se puede establecer como mapa por defecto del proyecto desde los Project Settings.

## 5. Ejecutar el proyecto

Una vez preparado el proyecto se puede ejecutar la simulación desde el editor de Unreal.

Para ello se debe utilizar la opción Play In Editor (PIE).

Al iniciar la ejecución del juego se activará el flujo de inicialización del framework.

Durante esta fase la GameInstance creará el Simulation Host y comenzará el proceso de inicialización de la simulación.

## 6. Proceso de inicialización

Durante el arranque del juego se ejecutan varias etapas de inicialización.

Primero se crea la GameInstance del proyecto. Esta instancia es responsable de iniciar el framework.

A continuación se crea el Simulation Host. Este componente se encarga de coordinar la inicialización de la simulación.

El host detecta la creación de un mundo válido y localiza el World Simulation Subsystem asociado a ese mundo.

Una vez localizado el subsystem se inicializan los componentes principales del runtime, como el Simulation Clock, el Scenario Runtime y el Message Bus.

Posteriormente se registran los systems activos y se construye el plan de ejecución de la simulación.

Si todas estas etapas se completan correctamente, el runtime pasa al estado de ejecución y comienza la simulación.

## 7. Ejecución de la simulación

Una vez iniciada la simulación, el runtime comienza a avanzar el estado del sistema utilizando pasos de tiempo fijo.

Durante cada frame del juego el reloj de simulación acumula el tiempo real transcurrido y determina cuántos pasos de simulación deben ejecutarse.

En cada paso se ejecutan los systems registrados en el runtime, que actualizan el estado de la simulación.

Este mecanismo permite mantener una ejecución determinista y garantiza que todos los systems operan sobre un estado coherente.

## 8. Verificación mediante logs

La forma más sencilla de comprobar que el ejemplo funciona correctamente es revisar los logs del sistema.

Durante la ejecución del proyecto deberían aparecer mensajes relacionados con:

- la inicialización del Simulation Host

- la creación del World Simulation Subsystem

- la inicialización del runtime de simulación

- el registro de systems

- la construcción del plan de ejecución

- el inicio de la simulación

Estos mensajes pueden observarse en la ventana Output Log del editor de Unreal.

Si todos los pasos se ejecutan sin errores, se puede confirmar que el framework está funcionando correctamente dentro del proyecto.

## 9. Resultado esperado

Si el ejemplo se ejecuta correctamente, el framework DRIVE se inicializará automáticamente cuando el juego comience.

El runtime de simulación se construirá y comenzará a ejecutarse utilizando pasos de tiempo fijo.

Aunque el mapa no contenga elementos visuales específicos de la simulación, el sistema estará activo y preparado para ejecutar los systems registrados.

Esto confirma que la infraestructura básica del framework está funcionando correctamente.

## 10. Próximos pasos

Una vez validado este ejemplo mínimo funcional, se puede comenzar a utilizar las piezas del framework para construir simulaciones más complejas.

Esto puede incluir la creación de nuevos systems, la integración con actores del mundo de juego o la implementación de lógica específica para los agentes de la simulación.

Para obtener más información sobre posibles problemas durante la ejecución del framework se recomienda consultar el documento [05_Troubleshooting.md](05_Troubleshooting.md), donde se describen algunos de los errores más comunes y sus posibles soluciones.
