// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GenericTeamAgentInterface.h"
#include "NPTeamSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API UNPTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// --- [1. 판사 역할: 관계 확인] ---

	// 두 액터의 팀 관계를 비교합니다. (같으면 Friendly, 다르면 Hostile)
	UFUNCTION(BlueprintCallable, Category = "NP|Team")
	ETeamAttitude::Type GetTeamAttitude(const AActor* AgentA, const AActor* AgentB) const;

	// 공격 가능한지 확인 (팀이 다르면 True)
	UFUNCTION(BlueprintCallable, Category = "NP|Team")
	bool CanCauseDamage(const AActor* Instigator, const AActor* Target, bool bAllowSelfDamage = false) const;


	// --- [2. 탐정 역할: ID 찾기] ---

	// 액터에게서 Team ID를 찾아냅니다. (Pawn -> Controller -> PlayerState 순으로 뒤짐)
	UFUNCTION(BlueprintCallable, Category = "NP|Team")
	int32 FindTeamID(const AActor* Agent) const;


	// --- [3. 관리자 역할: 팀 변경 및 등록] ---

	// 특정 액터의 팀을 변경합니다.
	UFUNCTION(BlueprintCallable, Category = "NP|Team")
	bool ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId);

	// (선택 사항) 현재 게임에 존재하는 팀 멤버 목록 관리
	// 키: TeamID, 값: 해당 팀에 소속된 액터들 목록
	// PvE에서 "남은 적의 수"를 세거나 "타겟팅 후보"를 찾을 때 유용합니다.
	void RegisterTeamMember(AActor* Member, int32 TeamId);
	void UnregisterTeamMember(AActor* Member, int32 TeamId);

private:
	// 팀 멤버 관리 맵 (메모리 관리를 위해 약한 참조 사용)
	TMap<int32, TArray<TWeakObjectPtr<AActor>>> TeamMemberMap;
};
