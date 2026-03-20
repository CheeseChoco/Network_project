// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NPAttributeSet.h"

// Sets default values
ANPCharacter::ANPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 1. 전투 시스템 장착
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UNPAttributeSet>(TEXT("AttributeSet"));
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

	if (AttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
			.AddUObject(this, &ANPCharacter::OnHealthChanged);
	}
	
}

void ANPCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	float newHealth = Data.NewValue;
	float oldHealth = Data.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("Ouch! HP changed from %f to %f"), OldHealth, NewHealth);

	// 3. 체력이 0보다 컸는데, 방금 데미지를 입고 0 이하가 된 경우에만 Die 호출
	if (oldHealth > 0.0f && newHealth <= 0.0f)
	{
		Die();
	}
}

void ANPCharacter::Die()
{
	// 플레이어와 적군이 공통으로 가지는 사망 로직 (필요 시 작성)
	UE_LOG(LogTemp, Log, TEXT("%s has died."), *GetName());
}

UAbilitySystemComponent* ANPCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGenericTeamId ANPCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}
