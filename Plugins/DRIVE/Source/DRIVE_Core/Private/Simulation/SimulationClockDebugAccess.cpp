#include "Simulation/FSimulationClock.h"

const FSimulationClock::FDiagnostics& DriveClockDebug_GetDiagnostics(const FSimulationClock& Clock)
{
    return Clock.GetDiagnostics();
}
