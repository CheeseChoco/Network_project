// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/NPCharacter.h"
#include "EnemyBase.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API AEnemyBase : public ANPCharacter
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;

public:
	AEnemyBase();

protected:


	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;
};
