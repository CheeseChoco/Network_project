// Fill out your copyright notice in the Description page of Project Settings.


#include "NPAttributeSet.h"
#include "GameplayEffectExtension.h"
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

void UNPAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 방금 수치가 변한 스탯이 'Damage(메타 어트리뷰트)'인지 확인합니다.
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 1. 임시 보관함(Damage)에 들어온 데미지 수치를 꺼냅니다. (예: 10)
		float LocalDamageDone = GetDamage();

		// 2. 메타 어트리뷰트는 일회용이므로, 다음 공격을 위해 즉시 0으로 비워줍니다.
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			// 3. 실제 체력(Health)에서 데미지를 뺍니다.
			// FMath::Clamp를 써서 체력이 MaxHealth를 넘거나 0 이하로 떨어지지 않게 막아줍니다.
			float NewHealth = FMath::Clamp(GetHealth() - LocalDamageDone, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			// (나중에 방어력(Armor) 스탯이 생긴다면, 여기서 깎기 전에 연산하면 됩니다!)
			// 예: float ActualDamage = LocalDamageDone - GetArmor();
		}
	}
}