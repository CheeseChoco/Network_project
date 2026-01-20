// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;


UCLASS()
class NETWORK_PROJECT_API ANPProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// 1. 충돌체 (동그라미 모양)
	// 투사체의 크기와 충돌 판정을 담당합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> SphereComp;

	// 2. 무브먼트 컴포넌트 (엔진)
	// 날아가는 속도, 중력, 유도 기능 등을 담당합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 초기 속도 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 1000.0f;

};
