// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PMCharacterBase.generated.h"

/** 플레이어와 몬스터가 공유하는 캐릭터 베이스입니다. 추후 GAS(ASC, AttributeSet)가 여기에 부착됩니다. */
UCLASS()
class PROJECTMETRUSH_API APMCharacterBase : public ACharacter
{
	GENERATED_BODY()

	// Character Interface
public:
	APMCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
};