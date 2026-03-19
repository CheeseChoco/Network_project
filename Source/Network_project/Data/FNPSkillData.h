// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FNPSkillData.generated.h"
/**
 * 
 */
class ANPProjectile;

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Projectile, // 투사체
	Area,       // 범위
	Target     // 즉발
};

USTRUCT(BlueprintType)
struct FNPSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category =	"GAS")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

public:
	// [1] 공통 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESkillType SkillType = ESkillType::Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ManaCost = 10.0f;

	// [2] 투사체 전용 데이터 (SkillType == Projectile일 때만 보임)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "SkillType == ESkillType::Projectile", EditConditionHides))
	TSubclassOf<ANPProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "SkillType == ESkillType::Projectile", EditConditionHides))
	float ProjectileSpeed = 1000.0f;

	// [3] 범위 전용 데이터 (SkillType == Area일 때만 보임)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "SkillType == ESkillType::Area", EditConditionHides))
	float Radius = 100.0f;
};
