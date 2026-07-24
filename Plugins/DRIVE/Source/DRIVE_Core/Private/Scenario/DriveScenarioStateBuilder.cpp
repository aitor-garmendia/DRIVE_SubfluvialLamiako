#include "Scenario/DriveScenarioStateBuilder.h"

static uint32 HashCombineStable(uint32 A, uint32 B)
{
	return A ^ (B + 0x9e3779b9u + (A << 6) + (A >> 2));
}

static uint32 HashNameStable(FName Name)
{
	return FCrc::StrCrc32(*Name.ToString());
}

static uint32 HashStringStable(const FString& S)
{
	return FCrc::StrCrc32(*S);
}

uint32 FDriveScenarioStateBuilder::ComputeDeterministicSeed(const FDriveScenarioDefinition& Def)
{
	uint32 Seed = HashNameStable(Def.Header.ScenarioId);
	Seed = HashCombineStable(Seed, (uint32)Def.Header.DefinitionVersion);

	// Systems
	{
		TArray<FName> Systems = Def.Systems.EnabledSystems;
		Systems.Sort([](const FName& A, const FName& B)
		{
			return A.LexicalLess(B);
		});

		for (const FName& Sys : Systems)
		{
			Seed = HashCombineStable(Seed, HashNameStable(Sys));
		}
	}

	// GlobalParams
	{
		TArray<FDriveScenarioGlobalParam> Params = Def.Global.GlobalParams;
		Params.Sort([](const FDriveScenarioGlobalParam& A, const FDriveScenarioGlobalParam& B)
		{
			return A.Key.LexicalLess(B.Key);
		});

		for (const auto& P : Params)
		{
			Seed = HashCombineStable(Seed, HashNameStable(P.Key));
			Seed = HashCombineStable(Seed, HashStringStable(P.Value));
		}
	}

	if (Def.Global.RandomSeed != 0)
	{
		Seed = HashCombineStable(Seed, (uint32)Def.Global.RandomSeed);
	}

	if (Seed == 0) Seed = 1;
	return Seed;
}

bool FDriveScenarioStateBuilder::BuildInitialState(const FDriveScenarioDefinition& Def, FDriveScenarioInitState& OutState)
{
	OutState = FDriveScenarioInitState{};

	if (!Def.IsValidBasic())
	{
		return false;
	}

	OutState.ScenarioId = Def.Header.ScenarioId;
	OutState.Seed = ComputeDeterministicSeed(Def);

	// Systems ordenados
	OutState.EnabledSystemsSorted = Def.Systems.EnabledSystems;
	OutState.EnabledSystemsSorted.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	// GlobalParams ordenados
	OutState.GlobalParamsSorted = Def.Global.GlobalParams;
	OutState.GlobalParamsSorted.Sort([](const FDriveScenarioGlobalParam& A, const FDriveScenarioGlobalParam& B)
	{
		return A.Key.LexicalLess(B.Key);
	});

	OutState.EntityTemplates = Def.EntityTemplates;
	OutState.Spawns = Def.Spawns;

	return OutState.IsValidBasic();
}
