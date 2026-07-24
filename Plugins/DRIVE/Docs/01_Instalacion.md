# INSTALACIÓN DE DRIVE


## 1. Introducción

Este documento describe el proceso de instalación e integración del framework DRIVE dentro de un proyecto de Unreal Engine.

El objetivo es poder integrar el plugin de manera sencilla y disponer de una simulación funcional con el mínimo número de pasos.

DRIVE está diseñado como un plugin modular que proporciona una infraestructura de simulación basada en ejecución determinista mediante fixed-step, una arquitectura de systems desacoplados, un runtime de simulación independiente del motor y una integración controlada con el ciclo de vida de Unreal Engine.

La instalación consiste principalmente en añadir el plugin al proyecto, compilarlo y configurar los ajustes globales necesarios para que la simulación pueda inicializarse correctamente.


## 2. Requisitos previos

Antes de instalar DRIVE se deben cumplir los siguientes requisitos.

### Unreal Engine

El proyecto debe estar basado en Unreal Engine 5 y debe tener soporte C++ habilitado. DRIVE no está diseñado para proyectos exclusivamente Blueprint.

### Compilación

Debe existir un entorno de compilación funcional, como Visual Studio configurado correctamente. La toolchain de Unreal debe estar instalada y el proyecto debe compilar correctamente antes de añadir el plugin.

### Estructura del proyecto

El proyecto debe permitir plugins dentro de la carpeta:

<ProjectRoot>/Plugins/

Si la carpeta Plugins no existe, puede crearse manualmente dentro de la raíz del proyecto.


## 3. Ubicación del plugin

El plugin DRIVE debe colocarse dentro de la carpeta Plugins del proyecto.

Ruta esperada: <ProjectRoot>/Plugins/DRIVE/

La estructura mínima del plugin debería ser similar a la siguiente:

ProjectRoot

├ Plugins

│ ├ DRIVE

│ ├ Source

│ │ ├ DRIVE_Core

│ │ ├ DRIVE_UE

│ │ └ DRIVE_UI

│ │

│ ├ Docs

│ └ README.md

│

└ TuProyecto.uproject

Una vez copiado el plugin en esta ubicación, Unreal Engine lo detectará automáticamente al abrir el proyecto.


## 4. Compilación del proyecto

Después de copiar el plugin, es necesario compilar el proyecto.

### Paso 1. Abrir el proyecto

Abrir el archivo TuProyecto.uproject. Unreal puede solicitar regenerar los archivos del proyecto. Si aparece este mensaje, se debe aceptar la regeneración.

### Paso 2. Generar archivos de proyecto (si es necesario)

Si el proyecto no genera automáticamente los archivos, se puede hacer manualmente haciendo clic derecho sobre el archivo .uproject y seleccionando la opción "Generate Visual Studio project files".

Esto generará la solución necesaria para compilar los módulos del plugin.

### Paso 3. Compilar

Abrir la solución generada en Visual Studio y compilar el proyecto.

Durante la compilación se construirán los módulos del plugin, entre ellos DRIVE_Core, DRIVE_UE y DRIVE_UI.

Si la compilación finaliza correctamente, el plugin estará disponible dentro del editor de Unreal.


## 5. Verificar que el plugin está cargado

Una vez compilado el proyecto se debe abrir el editor de Unreal Engine.

En el menú superior seleccionar Edit y después Plugins.

En la ventana de plugins buscar DRIVE. El plugin debe aparecer como activo.

Si el plugin no aparece, se debe revisar la ubicación del plugin, comprobar que la compilación no ha fallado y revisar los logs de Unreal para detectar posibles errores.


## 6. Configuración global del framework

DRIVE utiliza un sistema de configuración global accesible desde los Project Settings.

Esta configuración permite definir parámetros básicos de simulación como el paso fijo de simulación, la escala temporal o los límites de simulación.

Estos parámetros se almacenan en un Drive Global Config Asset.

### Acceder a la configuración

Dentro del editor de Unreal abrir el menú Edit y después Project Settings.

Buscar la sección correspondiente a DRIVE. En esta sección se debe asignar un asset de configuración global.

### Crear un Global Config Asset

Si aún no existe un asset de configuración global, se puede crear desde el Content Browser.

En el Content Browser crear un nuevo asset del tipo Drive Global Config Asset y guardarlo en una carpeta del proyecto, por ejemplo:

/Game/Drive/Config/

### Parámetros recomendados para pruebas

Para una configuración inicial se pueden utilizar los siguientes valores:

FixedDeltaTime = 0.0166667
DefaultTimeScale = 1.0
MaxFixedStepsPerFrame = 8

Estos valores permiten ejecutar la simulación a aproximadamente 60 pasos por segundo.


## 7. Configurar la GameInstance

DRIVE se inicializa automáticamente desde una GameInstance personalizada.

La implementación base incluida en el plugin es UDriveGameInstance.

Esta clase se encarga de crear el host de simulación, detectar la creación del mundo e inicializar el runtime de simulación.

### Asignar la GameInstance

Dentro del editor de Unreal abrir Edit, después Project Settings y posteriormente Maps & Modes.

En esta sección localizar el parámetro Game Instance Class.

Seleccionar una clase derivada de DriveGameInstance.

Esto permite que la simulación se inicialice automáticamente al arrancar el juego.


## 8. Inicialización de la simulación

Durante la ejecución del juego, el flujo de inicialización ocurre de la siguiente manera:

GameInstance
↓
DriveSimulationHost
↓
WorldSimulationSubsystem
↓
ScenarioRuntime
↓
Systems

El proceso es completamente automático.

### Flujo de arranque

Primero la GameInstance crea el Drive Simulation Host.
Después el host detecta la creación de un World válido.
A continuación se inicializa el WorldSimulationSubsystem.

Posteriormente se crean los componentes internos de simulación:

Simulation Clock
Scenario Runtime
Message Bus

Una vez creados estos componentes se registran los systems activos, se construye el execution plan y finalmente la simulación comienza a ejecutarse.


## 9. Verificación mediante logs

Para comprobar que la instalación es correcta se debe ejecutar el proyecto en modo PIE (Play In Editor).

En el Output Log deberían aparecer mensajes relacionados con la inicialización del host, la creación del subsystem, la inicialización del runtime, el registro de systems y el arranque de la simulación.

Si estos mensajes aparecen sin errores, la instalación se considera correcta.


## 10. Problemas comunes

### El plugin no aparece en Unreal

Las posibles causas pueden ser una ubicación incorrecta del plugin, una compilación fallida o módulos que no se han cargado correctamente.

### La simulación no se inicializa

En este caso se debe revisar la configuración de GameInstance, la configuración global del framework y los logs del sistema.

### El proyecto no compila

Se deben revisar las dependencias de módulos, las versiones de Unreal Engine utilizadas y posibles errores en los archivos Build.cs.

## 11. Siguiente paso

Una vez completada la instalación, se recomienda continuar con el documento [02_Piezas_Disponibles.md](02_Piezas_Disponibles.md)

Este documento describe las piezas principales del framework y su responsabilidad dentro de la arquitectura de DRIVE.
