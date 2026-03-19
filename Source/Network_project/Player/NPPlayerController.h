// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API ANPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ANPPlayerController();

protected:
	virtual void BeginPlay() override;

};
