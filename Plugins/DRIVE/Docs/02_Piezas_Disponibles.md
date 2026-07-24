# PIEZAS DISPONIBLES EN DRIVE

## 1. Introducción

Este documento describe las piezas principales que componen la arquitectura del framework DRIVE. El objetivo es proporcionar una visión general de los componentes disponibles y de la responsabilidad de cada uno dentro del sistema.

DRIVE está diseñado como un framework modular que separa claramente la lógica de simulación, el estado de simulación y la integración con Unreal Engine. Esta separación permite construir simulaciones complejas manteniendo un alto grado de desacoplamiento entre sistemas.

Las piezas del framework se agrupan en dos grandes categorías. Por un lado se encuentra el núcleo de simulación, que es independiente del motor gráfico. Por otro lado se encuentra la capa de integración con Unreal Engine, que permite ejecutar la simulación dentro del entorno del motor.

## 2. Núcleo de simulación

El núcleo de simulación contiene las piezas encargadas de ejecutar la simulación de forma determinista y controlada. Estas piezas no dependen directamente de Unreal Engine y pueden entenderse como la capa lógica del framework.

### Scenario Runtime

El Scenario Runtime constituye el núcleo de ejecución de la simulación. Su responsabilidad es mantener el estado del runtime y coordinar la ejecución de los distintos systems.

Entre sus funciones principales se encuentran la inicialización de la simulación, la validación del estado antes de arrancar, la ejecución del ciclo de simulación mediante pasos de tiempo fijo y la gestión del ciclo de vida del runtime.

El runtime mantiene distintos estados posibles, como detenido, en ejecución o pausado, y solo permite la ejecución de la simulación cuando se cumplen todas las condiciones necesarias.

### Simulation Clock

El Simulation Clock es el componente encargado de controlar el tiempo de simulación. Su función es convertir el tiempo real del sistema en pasos de simulación de tamaño fijo.

Este componente acumula el tiempo real transcurrido y determina cuántos pasos de simulación deben ejecutarse en cada frame. De esta manera se garantiza que la simulación se ejecuta con un paso temporal constante, independientemente de la tasa de frames del motor.

### Sim State

El Sim State actúa como contenedor del estado global de la simulación. En él se almacenan los datos que representan la situación actual del sistema simulado.

El objetivo de este componente es servir como fuente única de verdad para los datos de simulación. Todos los systems operan sobre este estado, lo que permite mantener coherencia y consistencia durante la ejecución.

### Message Bus

El Message Bus proporciona un mecanismo de comunicación desacoplada entre los distintos systems y componentes del runtime.

En lugar de establecer dependencias directas entre sistemas, los mensajes se envían a través del bus, que se encarga de distribuirlos a los destinatarios correspondientes. Este mecanismo facilita la extensibilidad del framework y reduce el acoplamiento entre componentes.

### Systems

Los systems representan las unidades funcionales que contienen la lógica de simulación. Cada system se encarga de una responsabilidad concreta dentro del sistema, como por ejemplo movimiento, gestión de agentes o lógica de comportamiento.

Los systems se registran en el runtime durante la fase de inicialización y posteriormente se ejecutan en cada paso de simulación. El runtime controla el orden de ejecución y garantiza que todos los systems operan sobre un estado consistente.

## 3. Integración con Unreal Engine

Además del núcleo de simulación, DRIVE incluye una capa de integración que permite ejecutar la simulación dentro de Unreal Engine.

Esta capa conecta el runtime de simulación con el ciclo de vida del motor y con los elementos del mundo de juego.

### Simulation Host

El Simulation Host actúa como punto de entrada principal del framework dentro de Unreal Engine. Su responsabilidad es coordinar la inicialización de la simulación y enlazar el runtime con el mundo de Unreal.

El host se encarga de cargar la configuración global, localizar el subsystem de simulación y construir el conjunto de systems activos que formarán parte de la simulación.

### World Simulation Subsystem

El World Simulation Subsystem es un subsystem asociado al mundo de Unreal. Su función es mantener las instancias del runtime, el reloj de simulación y el message bus dentro del contexto del mundo.

Este subsystem proporciona además funciones para iniciar, pausar o detener la simulación y permite que otros sistemas del proyecto interactúen con el runtime.

### Game Instance

La Game Instance se utiliza como punto de arranque del framework cuando el juego comienza a ejecutarse. Desde este punto se crea el Simulation Host y se inicia el proceso de inicialización de la simulación.

Esto permite que la simulación se configure automáticamente cuando el proyecto se ejecuta, sin necesidad de intervención manual por parte del usuario.

## 4. Configuración global

DRIVE incluye un sistema de configuración global que permite definir parámetros básicos del framework desde el editor de Unreal.

Estos parámetros se almacenan en un asset de configuración global y se exponen en los Project Settings.

Entre los parámetros configurables se encuentran el tamaño del paso de simulación, la escala temporal o distintos límites del sistema.

El objetivo de esta configuración es permitir ajustar el comportamiento global de la simulación sin necesidad de modificar el código.

## 5. Relación entre las piezas

La interacción entre las distintas piezas del framework sigue una estructura jerárquica clara.

Cuando el juego comienza, la Game Instance crea el Simulation Host. El host detecta la creación de un mundo válido y localiza el World Simulation Subsystem.

Una vez disponible el subsystem, se inicializan los componentes principales del runtime, como el Simulation Clock, el Scenario Runtime y el Message Bus.

Posteriormente se registran los systems activos y se construye el plan de ejecución de la simulación.

Durante la ejecución del juego, el runtime se encarga de avanzar la simulación en pasos temporales fijos, ejecutando los systems registrados en cada paso.

Este diseño permite mantener una clara separación entre la lógica de simulación y el motor gráfico, lo que facilita la escalabilidad y la reutilización del framework en distintos proyectos.

## 6. Resumen

DRIVE se compone de un conjunto de piezas claramente diferenciadas que trabajan conjuntamente para proporcionar una infraestructura de simulación modular y extensible.

El núcleo de simulación proporciona la lógica necesaria para ejecutar la simulación de forma determinista, mientras que la capa de integración permite conectar este núcleo con el entorno de Unreal Engine.

Esta arquitectura permite construir simulaciones complejas manteniendo un alto grado de desacoplamiento entre componentes y facilitando la evolución del framework en futuras versiones.

## 7. Siguiente paso

Una concluida la lectura de este documento, se recomienda continuar con el [03_Flujo_Recomendado.md](03_Flujo_Recomendado.md)
