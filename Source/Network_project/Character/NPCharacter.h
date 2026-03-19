// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "NPCharacter.generated.h"


UCLASS()
class NETWORK_PROJECT_API ANPCharacter : public APaperCharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCharacter();

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// [Team] 내 팀 명찰을 심판에게 보여주는 필수 인터페이스
	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UNPAttributeSet* AttributeSet;
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	uint8 TeamID = 255;


};
