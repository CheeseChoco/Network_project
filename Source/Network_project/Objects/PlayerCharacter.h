// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGameplayAbility;

UCLASS()
class NETWORK_PROJECT_API APlayerCharacter : public APaperCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;

	void UseSkill(); 

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	class UMyAttributeSet* AttributeSet;*/

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	void Move(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;


	// [1] 사용할 스킬 클래스 (블루프린트에서 GA_Fireball 넣을 변수)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> SkillAbilityClass;

	// [2] 입력 액션 (블루프린트에서 IA_Skill_Q 넣을 변수)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SkillAction;
};
