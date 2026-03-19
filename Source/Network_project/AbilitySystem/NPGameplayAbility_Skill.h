// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Network_project/Data/FNPSkillData.h"
#include "NPGameplayAbility_Skill.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API UNPGameplayAbility_Skill : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UNPGameplayAbility_Skill();

	// 스킬 발동 시 실행되는 메인 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// [핵심] 이 변수 덕분에 에디터에서 데이터 테이블의 행(Row)을 선택할 수 있습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Setting")
	FDataTableRowHandle SkillDataHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// 마우스 방향 계산 함수 (헬퍼)
	FVector GetDirectionToMouse(APlayerController* PC, FVector StartLocation);

};
