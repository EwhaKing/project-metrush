#include "ProjectMetrush/Component/PMStatComponent.h"
#include "Engine/Engine.h"

void UPMStatComponent::AcquireAugment(FName RowName, const FPMAugmentData& Data)
{
	// 이미 보유 중이면 강화 (최대 3단계, 수치만 상승)
	for (FPMOwnedAugment& Owned : OwnedAugments)
	{
		if (Owned.RowName == RowName)
		{
			Owned.Stage = FMath::Min(Owned.Stage + 1, 3);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
					FString::Printf(TEXT("증강 강화: %s → %d단계"), *Data.DisplayName.ToString(), Owned.Stage));
			}
			return;
		}
	}

	// 신규 획득
	FPMOwnedAugment NewAugment;
	NewAugment.RowName = RowName;
	NewAugment.Effect = Data.Effect;
	NewAugment.Stage = 1;
	NewAugment.StageValues = Data.StageValues;
	OwnedAugments.Add(NewAugment);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("증강 획득: %s (1단계)"), *Data.DisplayName.ToString()));
	}
}

bool UPMStatComponent::HasEffect(EPMAugmentEffect Effect) const
{
	for (const FPMOwnedAugment& Owned : OwnedAugments)
	{
		if (Owned.Effect == Effect)
		{
			return true;
		}
	}
	return false;
}

float UPMStatComponent::GetEffectValue(EPMAugmentEffect Effect) const
{
	for (const FPMOwnedAugment& Owned : OwnedAugments)
	{
		if (Owned.Effect == Effect && Owned.StageValues.IsValidIndex(Owned.Stage - 1))
		{
			return Owned.StageValues[Owned.Stage - 1];
		}
	}
	return 0.f;
}