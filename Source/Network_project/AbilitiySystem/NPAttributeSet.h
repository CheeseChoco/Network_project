// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NPAttributeSet.generated.h"

/**
 * 
 */

class AActor;
class ULyraAbilitySystemComponent;
class UObject;
class UWorld;
struct FGameplayEffectSpec;


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)



UCLASS()
class NETWORK_PROJECT_API UNPAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UNPAttributeSet();

	// 멀티플레이어 필수: 변수 동기화 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 유틸리티: 월드 정보 가져오기 (Lyra 스타일)
	virtual UWorld* GetWorld() const override;

	// -------------------------------------------------------------------
	//	Stats (스탯 정의)
	// -------------------------------------------------------------------

	// 1. 체력 (Current Health)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UNPAttributeSet, Health);

	// 2. 최대 체력 (Max Health)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UNPAttributeSet, MaxHealth);

	// 3. 공격력 (Attack Power)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UNPAttributeSet, AttackPower);

protected:
	// -------------------------------------------------------------------
	//	OnRep Functions (서버에서 값이 바뀌면 클라에서 실행됨)
	// -------------------------------------------------------------------

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

};
