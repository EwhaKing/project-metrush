// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PMCharacterBase.h"

APMCharacterBase::APMCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 베이스 캐릭터는 Tick을 사용하지 않습니다. (필요해지면 자식에서 켜기)
	PrimaryActorTick.bCanEverTick = false;
}

void APMCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}