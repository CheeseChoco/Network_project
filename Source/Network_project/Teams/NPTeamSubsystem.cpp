// Fill out your copyright notice in the Description page of Project Settings.


#include "NPTeamSubsystem.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemGlobals.h" // ASC 확인용

int32 UNPTeamSubsystem::FindTeamID(const AActor* Agent) const
{
	if (!Agent) return INDEX_NONE;

	// 1. IGenericTeamAgentInterface를 직접 구현한 경우 (캐릭터, AI 등)
	if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(Agent))
	{
		return TeamAgent->GetGenericTeamId().GetId();
	}

	// 2. Pawn인 경우 -> Controller나 PlayerState를 뒤져본다.
	if (const APawn* Pawn = Cast<const APawn>(Agent))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			// 컨트롤러가 팀 인터페이스를 가졌나?
			if (const IGenericTeamAgentInterface* ControllerTeam = Cast<const IGenericTeamAgentInterface>(Controller))
			{
				return ControllerTeam->GetGenericTeamId().GetId();
			}
			// PlayerState가 팀 인터페이스를 가졌나? (Lyra 방식)
			if (Controller->PlayerState)
			{
				if (const IGenericTeamAgentInterface* PSTeam = Cast<const IGenericTeamAgentInterface>(Controller->PlayerState))
				{
					return PSTeam->GetGenericTeamId().GetId();
				}
			}
		}
	}

	// 3. 투사체(Projectile)인 경우 -> Instigator(발사자)를 추적
	if (const AActor* Instigator = Agent->GetInstigator())
	{
		return FindTeamID(Instigator);
	}

	return INDEX_NONE; // 팀 없음
}

ETeamAttitude::Type UNPTeamSubsystem::GetTeamAttitude(const AActor* AgentA, const AActor* AgentB) const
{
	if (!AgentA || !AgentB) return ETeamAttitude::Neutral;

	int32 TeamA = FindTeamID(AgentA);
	int32 TeamB = FindTeamID(AgentB);

	// 둘 중 하나라도 팀이 없으면 -> 일단 적대적 or 중립 (기획에 따라 변경)
	if (TeamA == INDEX_NONE || TeamB == INDEX_NONE)
	{
		return ETeamAttitude::Neutral;
	}

	// [핵심 판정] 팀 ID가 다르면 적이다!
	return (TeamA != TeamB) ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}

bool UNPTeamSubsystem::CanCauseDamage(const AActor* Instigator, const AActor* Target, bool bAllowSelfDamage) const
{
	if (!Instigator || !Target) return false;

	// 자해 불가 체크
	if (!bAllowSelfDamage && (Instigator == Target))
	{
		return false;
	}

	// 팀 관계 확인
	ETeamAttitude::Type Attitude = GetTeamAttitude(Instigator, Target);

	// 적대적이면 공격 허용
	return Attitude == ETeamAttitude::Hostile;
}

bool UNPTeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId)
{
	if (!ActorToChange) return false;

	// 기존 팀에서 제거
	int32 OldTeamId = FindTeamID(ActorToChange);
	UnregisterTeamMember(ActorToChange, OldTeamId);

	// 팀 변경 시도 (인터페이스 사용)
	if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(ActorToChange))
	{
		TeamAgent->SetGenericTeamId(FGenericTeamId(NewTeamId));

		// 새 팀에 등록
		RegisterTeamMember(ActorToChange, NewTeamId);
		return true;
	}

	// Pawn이라면 PlayerState의 팀을 바꿔줘야 함
	if (APawn* Pawn = Cast<APawn>(ActorToChange))
	{
		if (Pawn->GetPlayerState())
		{
			// PlayerState도 인터페이스를 구현해야 함!
			if (IGenericTeamAgentInterface* PSInterface = Cast<IGenericTeamAgentInterface>(Pawn->GetPlayerState()))
			{
				PSInterface->SetGenericTeamId(FGenericTeamId(NewTeamId));
				RegisterTeamMember(ActorToChange, NewTeamId);
				return true;
			}
		}
	}

	return false;
}

void UNPTeamSubsystem::RegisterTeamMember(AActor* Member, int32 TeamId)
{
	if (Member && TeamId != INDEX_NONE)
	{
		TeamMemberMap.FindOrAdd(TeamId).AddUnique(Member);
	}
}

void UNPTeamSubsystem::UnregisterTeamMember(AActor* Member, int32 TeamId)
{
	if (TeamId != INDEX_NONE)
	{
		if (TArray<TWeakObjectPtr<AActor>>* List = TeamMemberMap.Find(TeamId))
		{
			List->Remove(Member);
		}
	}
}
