# TROUBLESHOOTING

## 1. Introducción

Este documento describe algunos de los problemas más comunes que pueden aparecer al instalar o utilizar el framework DRIVE dentro de un proyecto de Unreal Engine.

El objetivo es proporcionar una guía rápida para identificar posibles errores durante la inicialización o ejecución del sistema y ofrecer algunas comprobaciones básicas que permitan resolverlos.

La mayoría de los problemas suelen estar relacionados con la configuración del proyecto, la inicialización del framework o la compilación de los módulos del plugin.

## 2. El plugin no aparece en Unreal

Uno de los problemas más habituales es que el plugin DRIVE no aparece en la lista de plugins disponibles dentro del editor de Unreal.

Las causas más comunes de este problema son las siguientes:

El plugin no está ubicado en la carpeta correcta del proyecto.

La carpeta DRIVE debe encontrarse dentro de la ruta: <ProjectRoot>/Plugins/DRIVE/

Si la carpeta del plugin está en otra ubicación, Unreal Engine no podrá detectarla.

Otra posible causa es que el proyecto no se haya recompilado después de añadir el plugin. En este caso es necesario generar los archivos de proyecto y compilar nuevamente.

También es posible que exista algún error en la compilación de los módulos del plugin, lo que impide que Unreal los cargue correctamente.

## 3. El proyecto no compila

Si el proyecto no compila después de añadir el plugin, es probable que exista un problema en la configuración del entorno de compilación o en las dependencias del proyecto.

Las comprobaciones recomendadas son las siguientes:

Verificar que Visual Studio está correctamente instalado y configurado.

Comprobar que la versión de Unreal Engine utilizada es compatible con el plugin.

Revisar los archivos Build.cs de los módulos del plugin para asegurarse de que todas las dependencias necesarias están incluidas.

También se recomienda revisar los mensajes de error que aparecen durante la compilación, ya que suelen indicar la causa exacta del problema.

## 4. El framework no se inicializa al ejecutar el juego

Si el proyecto se ejecuta pero el framework DRIVE no se inicializa, es posible que el problema esté relacionado con la configuración de la GameInstance.

Para que la simulación se inicialice automáticamente, el proyecto debe utilizar una GameInstance derivada de DriveGameInstance.

Esto se configura desde los Project Settings, en la sección Maps & Modes, dentro del parámetro Game Instance Class.

Si el proyecto utiliza una GameInstance diferente, el Simulation Host no se creará automáticamente y la simulación no comenzará.

## 5. El World Simulation Subsystem no se crea

El World Simulation Subsystem es el componente encargado de mantener las instancias del runtime de simulación dentro del contexto del mundo de Unreal.

Si este subsystem no se crea correctamente, la simulación no podrá inicializarse.

Una posible causa es que el mundo en el que se está ejecutando el proyecto no sea un mundo de tipo Game o PIE.

Los subsystems de mundo solo se crean cuando el juego se ejecuta realmente. Si el proyecto está simplemente abierto en el editor sin ejecutar el juego, el subsystem no existirá.

Para comprobar que el subsystem se está creando correctamente se pueden revisar los logs del sistema durante el arranque del juego.

## 6. La simulación no comienza a ejecutarse

En algunos casos el framework puede inicializarse correctamente pero la simulación no comienza a ejecutarse.

Esto puede ocurrir si el runtime no puede pasar al estado de ejecución.

Las posibles causas incluyen problemas durante la inicialización del runtime, errores en la creación de los systems o fallos en la construcción del plan de ejecución.

Para diagnosticar este problema se recomienda revisar los mensajes del Output Log y comprobar si existen advertencias o errores relacionados con el runtime.

## 7. Los systems no se ejecutan

Si la simulación se inicia pero los systems no parecen ejecutarse, puede existir un problema en el registro de systems dentro del runtime.

Durante la inicialización de la simulación el Simulation Host debe crear y registrar los systems activos.

Si este proceso falla o si no se registran systems, el runtime no tendrá lógica de simulación que ejecutar.

Se recomienda comprobar que el proceso de registro de systems se realiza correctamente durante la fase de inicialización.

## 8. Problemas relacionados con el tiempo de simulación

Si la simulación se ejecuta de forma irregular o no parece avanzar correctamente, el problema puede estar relacionado con la configuración del reloj de simulación.

El Simulation Clock utiliza un paso de tiempo fijo para ejecutar la simulación.

Si el valor de FixedDeltaTime es demasiado grande o demasiado pequeño, el comportamiento de la simulación puede verse afectado.

También es posible que el límite de pasos por frame sea demasiado bajo, lo que puede impedir que la simulación recupere el tiempo acumulado.

Estos parámetros pueden ajustarse desde el asset de configuración global del framework.

## 9. Revisar los logs del sistema

La forma más efectiva de diagnosticar problemas en DRIVE es revisar los logs del sistema.

El Output Log del editor de Unreal muestra información detallada sobre el proceso de inicialización del framework y sobre la ejecución de la simulación.

Durante el arranque del juego deberían aparecer mensajes relacionados con la creación del Simulation Host, la inicialización del World Simulation Subsystem, la construcción del runtime de simulación y el registro de systems.

Si aparece algún error o advertencia, este suele indicar el origen del problema.

## 10. Resumen

La mayoría de los problemas relacionados con DRIVE se deben a errores de configuración o a problemas durante la compilación del proyecto.

Revisar la ubicación del plugin, la configuración de la GameInstance, los Project Settings y los logs del sistema suele ser suficiente para identificar y resolver la mayoría de incidencias.

Si el framework está correctamente instalado y configurado, el proceso de inicialización de la simulación debería ejecutarse automáticamente al iniciar el juego.