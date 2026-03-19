// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCharacter.h"
#include "AbilitySystemComponent.h"

// Sets default values
ANPCharacter::ANPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 1. 전투 시스템 장착
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
}

// Called when the game starts or when spawned
void ANPCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// GAS 초기화: 나(Owner)와 아바타(Avatar)가 누군지 알려줌
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
}

UAbilitySystemComponent* ANPCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGenericTeamId ANPCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}
