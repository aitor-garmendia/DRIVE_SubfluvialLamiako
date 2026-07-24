#include "AnimGraphNode_CopyPoseRotations.h"
#include "Animation/AnimAttributes.h"

#define LOCTEXT_NAMESPACE "AnimGraphNode_CopyPoseRotations"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UAnimGraphNode_CopyPoseRotations
UAnimGraphNode_CopyPoseRotations::UAnimGraphNode_CopyPoseRotations(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_CopyPoseRotations::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_CopyPoseRotations_Tooltip", "The Copy Pose Rotations node copies the pose rotation data from another component to this. Only works when name matches.");
}

FText UAnimGraphNode_CopyPoseRotations::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("CopyPoseRotations", "Copy Pose Rotations");
}

void UAnimGraphNode_CopyPoseRotations::GetOutputLinkAttributes(FNodeAttributeArray& OutAttributes) const
{
	OutAttributes.Add(UE::Anim::FAttributes::Curves);
	OutAttributes.Add(UE::Anim::FAttributes::Attributes);
}

#undef LOCTEXT_NAMESPACE // AnimGraphNode_CopyPoseRotations
