// Fill out your copyright notice in the Description page of Project Settings.


#include "NPProjectile.h"
#include "Components/SphereComponent.h"
#include "Teams/NPTeamSubsystem.h" 
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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

	ProjectileMovement->bConstrainToPlane = true;
	ProjectileMovement->SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f)); // Y축을 기준으로 막음	
}

// Called when the game starts or when spawned
void ANPProjectile::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("Projectile Begin"));

	// 3초 뒤 자동 소멸 (메모리 누수 방지)
	SetLifeSpan(3.0f);
}

void ANPProjectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);


	// 1. 기본 방어 코드 (나 자신, 나를 쏜 주인, 빈 공간 제외)
	if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this)
	{
		return;
	}

	// 2. 심판(TeamSubsystem) 호출
	UNPTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UNPTeamSubsystem>();
	if (!TeamSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("TeamSubsystem을 찾을 수 없습니다!"));
		return;
	}

	// 3. 판결: "얘, 제가 때려도 되는 적인가요?"
	if (TeamSubsystem->CanCauseDamage(GetInstigator(), OtherActor))
	{
		// === 적이 맞다! ===
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *OtherActor->GetName());


		// 4. 상대방의 ASC(Ability System Component) 찾기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

		if (DamageEffectSpecHandle.IsValid()) {
			UE_LOG(LogTemp, Error, TEXT("handle"));
		}

		// 서버에서만 데미지를 처리하고 명세서가 유효한지 확인
		if (TargetASC && HasAuthority() && DamageEffectSpecHandle.IsValid())
		{
			// 5. [GAS 정석] 데미지 명세서를 적에게 적용 (체력 깎기)
			TargetASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);

			UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *OtherActor->GetName());
		}

		// 6. 할 일 다 했으니 자폭
		Destroy();
	}
	else
	{
		// === 적이 아니다! (아군이거나 그냥 벽임) ===

		// 캐릭터가 아니라 그냥 배경(벽, 바닥)에 부딪힌 거라면 터져야 함
		// ASC가 없다면 무생물(벽)으로 간주하고 파괴
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (!TargetASC)
		{
			Destroy();
		}
		// 아군(ASC가 있는데 팀이 같음)이라면 터지지 않고 관통하게 둘 수도 있음 (기획에 따라 다름)
		// 관통시키기 싫으면 여기도 Destroy() 추가.
	}
}