#pragma once

#include "CoreMinimal.h"

// --------------------
// Categorías
// --------------------
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveCore, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveRuntime, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveClock, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveMessageBus, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveHost, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveUE, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveSimState, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveSystems, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveDebug, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveManager, Log, All);
DRIVE_CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDriveTest, Log, All);

// ------------------------------
// Toggle global (compile-time)
// ------------------------------
#ifndef DRIVE_ENABLE_LOGGING
#define DRIVE_ENABLE_LOGGING 1
#endif

// --------------------
// Macro base
// --------------------
#if DRIVE_ENABLE_LOGGING
	#define DRIVE_LOG(Category, Verbosity, Format, ...) \
		UE_LOG(Category, Verbosity, Format, ##__VA_ARGS__)
#else
	#define DRIVE_LOG(Category, Verbosity, Format, ...) \
		do { } while(0)
#endif

// ------------------------------
// Wrappers por categoría
// ------------------------------
#define DRIVE_LOG_CORE(Verbosity, Format, ...)       DRIVE_LOG(LogDriveCore, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_RUNTIME(Verbosity, Format, ...)    DRIVE_LOG(LogDriveRuntime, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_CLOCK(Verbosity, Format, ...)      DRIVE_LOG(LogDriveClock, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_BUS(Verbosity, Format, ...)        DRIVE_LOG(LogDriveMessageBus, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_HOST(Verbosity, Format, ...)       DRIVE_LOG(LogDriveHost, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_UE(Verbosity, Format, ...)         DRIVE_LOG(LogDriveUE, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_SIMSTATE(Verbosity, Format, ...)   DRIVE_LOG(LogDriveSimState, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_SYSTEMS(Verbosity, Format, ...)    DRIVE_LOG(LogDriveSystems, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_DEBUG(Verbosity, Format, ...)    	 DRIVE_LOG(LogDriveDebug, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_MANAGER(Verbosity, Format, ...)    DRIVE_LOG(LogDriveManager, Verbosity, Format, ##__VA_ARGS__)
#define DRIVE_LOG_TEST(Verbosity, Format, ...)    	 DRIVE_LOG(LogDriveTest, Verbosity, Format, ##__VA_ARGS__)