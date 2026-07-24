#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

class FDriveEcsWorld;

/**
 * Cola de comandos diferidos.
 */
class DRIVE_CORE_API FDriveEcsCommandBuffer
{
public:
	void Enqueue(TFunction<void(FDriveEcsWorld&)>&& Cmd)
	{
		Commands.Emplace(MoveTemp(Cmd));
	}

	bool HasPending() const { return Commands.Num() > 0; }

	int32 Flush(FDriveEcsWorld& World)
	{
		const int32 Count = Commands.Num();
		for (TFunction<void(FDriveEcsWorld&)>& Cmd : Commands)
		{
			Cmd(World);
		}
		Commands.Reset();
		return Count;
	}

	void Reset()
	{
		Commands.Reset();
	}

private:
	TArray<TFunction<void(FDriveEcsWorld&)>> Commands;
};
