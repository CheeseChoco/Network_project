// Fill out your copyright notice in the Description page of Project Settings.


#include "NPAttributeSet.h"
#include "Net/UnrealNetwork.h"  // DOREPLIFETIME 매크로 필수
#include "Engine/World.h"


UNPAttributeSet::UNPAttributeSet()
{
	// 기본값 초기화 (나중에는 데이터 테이블 연동 가능)
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitAttackPower(10.0f);
}

void UNPAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 변수들을 네트워크로 공유하겠다고 등록 (REPNOTIFY_Always: 값이 같아도 알림 발생)
	DOREPLIFETIME_CONDITION_NOTIFY(UNPAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNPAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNPAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
}

UWorld* UNPAttributeSet::GetWorld() const
{
	// 내 주인(Outer)을 찾아서 월드를 물어봄 (안전장치)
	const UObject* Outer = GetOuter();
	if (Outer)
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

void UNPAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNPAttributeSet, Health, OldHealth);
}

void UNPAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNPAttributeSet, MaxHealth, OldMaxHealth);
}

void UNPAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNPAttributeSet, AttackPower, OldAttackPower);
}