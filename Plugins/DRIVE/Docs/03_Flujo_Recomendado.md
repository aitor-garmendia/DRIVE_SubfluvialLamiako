# FLUJO RECOMENDADO DE USO

## 1. Introducción

Este documento describe el flujo recomendado para utilizar el framework DRIVE dentro de un proyecto de Unreal Engine.

El objetivo es proporcionar una guía clara que permita integrar y ejecutar una simulación utilizando las piezas disponibles del framework. El flujo descrito sigue el ciclo de vida previsto por la arquitectura de DRIVE y garantiza que todos los componentes del sistema se inicializan y ejecutan en el orden correcto.

Siguiendo este flujo se puede conseguir una simulación funcional con un número reducido de pasos y sin necesidad de modificar el núcleo del framework.

## 2. Visión general del flujo

El proceso de uso de DRIVE puede dividirse en varias etapas principales:

1. Instalación del plugin en el proyecto.

2. Configuración global del framework.

3. Configuración de la GameInstance.

4. Inicialización automática de la simulación.

5. Registro de systems activos.

6. Construcción del plan de ejecución.

7. Ejecución de la simulación en pasos de tiempo fijo.

Cada una de estas etapas forma parte del ciclo de vida normal del sistema.

## 3. Paso 1: Instalar el plugin

El primer paso consiste en integrar el plugin DRIVE dentro del proyecto de Unreal Engine.

Para ello se debe copiar la carpeta del plugin dentro de la ruta: <ProjectRoot>/Plugins/DRIVE/

Una vez copiado el plugin, el proyecto debe compilarse para que Unreal Engine pueda cargar los módulos correspondientes.

Después de la compilación, el plugin aparecerá disponible dentro del editor de Unreal.

## 4. Paso 2: Configurar la configuración global

Una vez instalado el plugin, se debe configurar el asset de configuración global del framework.

Este asset define parámetros básicos del comportamiento de la simulación, como el paso de tiempo fijo o los límites de ejecución.

Para configurar estos parámetros se debe acceder a los Project Settings dentro del editor de Unreal y localizar la sección correspondiente a DRIVE.

En esta sección se debe asignar un Drive Global Config Asset válido.

Este asset puede crearse desde el Content Browser y almacenarse dentro del contenido del proyecto.

## 5. Paso 3: Configurar la GameInstance

El framework DRIVE utiliza una GameInstance personalizada para inicializar la simulación cuando el juego comienza.

La clase base incluida en el plugin es UDriveGameInstance.

Esta clase se encarga de crear el host de simulación y de detectar la creación de un mundo válido donde la simulación pueda ejecutarse.

Para utilizar esta funcionalidad se debe asignar una clase derivada de DriveGameInstance en los Project Settings del proyecto, dentro de la sección Maps & Modes.

Una vez configurada la GameInstance, el proceso de inicialización de la simulación se realizará automáticamente al ejecutar el juego.

## 6. Paso 4: Inicialización del sistema de simulación

Cuando el juego comienza a ejecutarse, la GameInstance crea el Simulation Host.

El host es el componente encargado de coordinar la inicialización del sistema de simulación.

Durante esta fase el host detecta la creación de un mundo válido y localiza el World Simulation Subsystem asociado a dicho mundo.

Una vez localizado el subsystem, se inicializan los componentes principales del runtime de simulación.

Entre estos componentes se encuentran el reloj de simulación, el runtime de escenarios y el sistema de mensajería interno.

## 7. Paso 5: Registro de systems

Después de inicializar el runtime, el host construye el conjunto de systems activos que formarán parte de la simulación.

Los systems representan las unidades funcionales que contienen la lógica de simulación. Cada system se encarga de una responsabilidad concreta dentro del sistema.

Una vez creados, los systems se registran en el runtime y quedan preparados para ser inicializados y ejecutados durante la simulación.

## 8. Paso 6: Construcción del plan de ejecución

Una vez registrados los systems, el runtime construye el plan de ejecución de la simulación.

Este plan determina el orden en el que se ejecutarán los systems durante cada paso de simulación.

Durante esta fase también se inicializan los systems y se valida que el estado del runtime es correcto para iniciar la simulación.

Si todas las validaciones se completan correctamente, la simulación queda preparada para comenzar.

## 9. Paso 7: Ejecución de la simulación

Una vez construido el plan de ejecución, el runtime pasa al estado de ejecución.

Durante cada frame del juego, el reloj de simulación acumula el tiempo real transcurrido y determina cuántos pasos de simulación deben ejecutarse.

El runtime ejecuta entonces los systems registrados utilizando pasos de tiempo fijo.

Este mecanismo garantiza que la simulación se ejecuta de manera determinista y que todos los systems operan sobre un estado consistente.

## 10. Interacción con la simulación

Durante la ejecución del juego, otros componentes del proyecto pueden interactuar con la simulación a través del World Simulation Subsystem o mediante los managers y actores proporcionados por el framework.

Esto permite integrar la simulación con elementos del mundo de juego, como agentes, actores visuales o sistemas de control.

Gracias a la arquitectura desacoplada de DRIVE, estos elementos pueden interactuar con la simulación sin depender directamente de la lógica interna del runtime.

## 11. Resumen del flujo

El flujo completo de uso del framework puede resumirse de la siguiente manera:

El plugin se instala dentro del proyecto y se compila.
Se configura el asset de configuración global del framework.
Se asigna una GameInstance derivada de DriveGameInstance.
Al iniciar el juego, el Simulation Host se crea automáticamente.
El host localiza el World Simulation Subsystem.
Se inicializa el runtime de simulación.
Se registran los systems activos.
Se construye el plan de ejecución.
La simulación comienza a ejecutarse utilizando pasos de tiempo fijo.

Siguiendo este flujo es posible integrar y ejecutar una simulación funcional utilizando la infraestructura proporcionada por DRIVE.

## 12. Siguiente paso

Una vez comprendido el flujo recomendado de uso, se recomienda continuar con el documento [04_Ejemplo_Minimo_Funcional.md](04_Ejemplo_Minimo_Funcional.md)
