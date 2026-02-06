// Fill out your copyright notice in the Description page of Project Settings.


#include "NPGameplayAbility_Skill.h"
#include "Network_project/Objects/NPProjectile.h"
#include "Network_project/Data/FNPSkillData.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UNPGameplayAbility_Skill::UNPGameplayAbility_Skill()
{
	// GA 설정: 인스턴싱 (스킬 쓸 때마다 객체 생성)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UNPGameplayAbility_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 1. 쿨타임 및 자원 체크 (Commit)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 데이터 테이블 읽기 (연결의 핵심!)
	// "Context"는 에러 로그에 찍힐 식별자입니다.
	FNPSkillData* SkillData = SkillDataHandle.GetRow<FNPSkillData>(TEXT("Skill Ability Context"));

	if (!SkillData)
	{
		UE_LOG(LogTemp, Error, TEXT("데이터 테이블이 비어있거나, Row Name이 잘못되었습니다!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 스킬 타입에 따른 분기 처리
	if (SkillData->SkillType == ESkillType::Projectile)
	{
		// --- 투사체 발사 로직 ---
		APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get());
		if (PC && SkillData->ProjectileClass)
		{
			// 시작 위치 (캐릭터 위치)
			FVector SpawnLocation = ActorInfo->AvatarActor->GetActorLocation();

			// 방향 계산
			FVector Direction = GetDirectionToMouse(PC, SpawnLocation);
			FRotator SpawnRotation = Direction.Rotation();

			// 서버 권한 확인 (투사체는 서버에서 소환해야 함)
			if (HasAuthority(&ActivationInfo))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = ActorInfo->AvatarActor.Get();
				SpawnParams.Instigator = Cast<APawn>(ActorInfo->AvatarActor.Get());

				// [진짜 소환] 데이터 테이블에 있는 클래스를 소환
				ANPProjectile* Projectile = GetWorld()->SpawnActor<ANPProjectile>(
					SkillData->ProjectileClass,
					SpawnLocation,
					SpawnRotation,
					SpawnParams
				);

				// [속도 덮어쓰기] 데이터 테이블에 적힌 속도로 변경
				if (Projectile && Projectile->ProjectileMovement)
				{
					Projectile->ProjectileMovement->InitialSpeed = SkillData->ProjectileSpeed;
					Projectile->ProjectileMovement->MaxSpeed = SkillData->ProjectileSpeed;

					// (추후 구현) 데미지 정보도 투사체에 넘겨주면 됩니다.
					// Projectile->Damage = SkillData->Damage; 
				}
			}
		}
	}
	else if (SkillData->SkillType == ESkillType::Area)
	{
		// 범위 스킬 로직 (나중에 구현)
		UE_LOG(LogTemp, Log, TEXT("범위 스킬 발동! 반경: %f"), SkillData->Radius);
	}

	// 4. 스킬 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

FVector UNPGameplayAbility_Skill::GetDirectionToMouse(APlayerController* PC, FVector StartLocation)
{
	FVector WorldLoc, WorldDir;
	if (PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		// 2D 횡스크롤 기준 Y=0 평면 교차점 계산
		// 만약 Top-Down 게임이라면 Z=0 (혹은 캐릭터 높이) 평면으로 바꿔야 합니다.
		float TargetY = 0.0f;

		// 0으로 나누기 방지
		if (FMath::IsNearlyZero(WorldDir.Y)) return FVector::ForwardVector;

		float t = (TargetY - WorldLoc.Y) / WorldDir.Y;
		FVector TargetPos = WorldLoc + (WorldDir * t);

		return (TargetPos - StartLocation).GetSafeNormal();
	}

	// 마우스 위치 못 찾으면 그냥 캐릭터 정면으로 발사
	return PC->GetPawn() ? PC->GetPawn()->GetActorForwardVector() : FVector::ForwardVector;
}