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
				FGameplayEffectSpecHandle DamageSpecHandle;
				if (SkillData->DamageEffectClass)
				{
					// 내 능력(Ability)을 기반으로 GE 클래스에서 명세서를 찍어냅니다.
					DamageSpecHandle = MakeOutgoingGameplayEffectSpec(SkillData->DamageEffectClass, GetAbilityLevel());

					FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

					if (DamageSpecHandle.IsValid())
					{
						// 데이터 테이블의 Damage 수치를 GE 명세서에 찔러 넣습니다!
						DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, SkillData->Damage);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("데이터 테이블에 DamageEffectClass가 비어있습니다!"));
				}



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

				Projectile->DamageEffectSpecHandle = DamageSpecHandle;

				// [속도 덮어쓰기] 데이터 테이블에 적힌 속도로 변경
				if (Projectile && Projectile->ProjectileMovement)
				{

					Projectile->ProjectileMovement->InitialSpeed = SkillData->ProjectileSpeed;
					Projectile->ProjectileMovement->MaxSpeed = SkillData->ProjectileSpeed;


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

	// 1. 마우스 위치를 3D 월드 좌표와 방향으로 변환
	if (PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		// [수정됨] 탑다운 게임은 바닥(Z=0) 평면과 교차점을 찾아야 함.

		// 카메라가 바닥을 절대 안 보는 상황(완전 수평) 예외 처리
		if (FMath::IsNearlyZero(WorldDir.Z))
		{
			return PC->GetPawn() ? PC->GetPawn()->GetActorForwardVector() : FVector::ForwardVector;
		}

		// 2. 마우스 레이저가 바닥(Z=0)에 닿는 거리(t) 계산
		// 공식: (목표높이 - 현재높이) / 기울기
		float t = (0.0f - WorldLoc.Z) / WorldDir.Z;

		// 3. 실제 바닥 좌표 계산
		FVector MouseGroundLocation = WorldLoc + (WorldDir * t);

		// 4. 방향 벡터 계산 (목표점 - 시작점)
		FVector Direction = MouseGroundLocation - StartLocation;

		// [중요] 높낮이(Z) 무시하고 수평으로만 쏘기
		// 이걸 안 하면 캐릭터 손에서 발등으로 꽂히는 샷이 나갑니다.
		Direction.Z = 0.0f;

		return Direction.GetSafeNormal();
	}

	return FVector::ForwardVector;
}