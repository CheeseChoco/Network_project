// Fill out your copyright notice in the Description page of Project Settings.


#include "NPProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ANPProjectile::ANPProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌체 설정
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(15.0f);
	// 나중에 "Projectile" 프리셋을 만드셔야 합니다. (일단은 기본 OverlapAllDynamic 등으로 설정됨)
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComp;

	// 2. 무브먼트 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereComp;
	ProjectileMovement->InitialSpeed = MoveSpeed;
	ProjectileMovement->MaxSpeed = MoveSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향으로 회전
	ProjectileMovement->bShouldBounce = false; // 튕기기 끔

	// [핵심] 중력 제거 (직사 화염구)
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// [핵심] 2D 횡스크롤을 위한 Y축(깊이) 고정
	// 이걸 켜면 물리 엔진이 강제로 Y축 이동을 막아버립니다. (Paper2D 필수 설정)
	ProjectileMovement->bConstrainToPlane = true;
	ProjectileMovement->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f)); // Y축을 기준으로 막음
	ProjectileMovement->SetPlaneConstraintOrigin(FVector(0.0f, 0.0f, 0.0f)); // Y=0 평면 위에서만 놀아라
}

// Called when the game starts or when spawned
void ANPProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 3초 뒤 자동 소멸 (메모리 누수 방지)
	SetLifeSpan(3.0f);
}
