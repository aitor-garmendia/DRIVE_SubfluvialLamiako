#pragma once

#include "CoreMinimal.h"

struct FDriveTransformComponent
{
	FTransform Transform = FTransform::Identity;

	FVector GetLocation() const { return Transform.GetLocation(); }
	void SetLocation(const FVector& InLocation) { Transform.SetLocation(InLocation); }

	void AddWorldOffset(const FVector& Delta) { Transform.AddToTranslation(Delta); }
};

struct FDriveVelocityComponent
{
	FVector Linear = FVector::ZeroVector;
	FVector Angular = FVector::ZeroVector;
};

struct FDriveTagsComponent
{
	TSet<FName> Tags;

	bool HasTag(const FName& Tag) const { return Tags.Contains(Tag); }
	void AddTag(const FName& Tag) { Tags.Add(Tag); }
	void RemoveTag(const FName& Tag) { Tags.Remove(Tag); }
};
