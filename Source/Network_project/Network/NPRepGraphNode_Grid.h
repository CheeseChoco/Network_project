// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "NPRepGraphNode_Grid.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API UNPRepGraphNode_Grid : public UReplicationGraphNode_GridSpatialization2D
{
	GENERATED_BODY()
	
public:
	UNPRepGraphNode_Grid();

	// 리플리케이션 업데이트 루프의 핵심 함수
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound = true) override;
	virtual void NotifyResetAllNetworkActors() override;


protected:
	// 시각화를 위해 '현재 맵에 존재하는 전체 동적 액터'를 추적하는 명부
	// 엔진 내장 노드에도 리스트가 있지만 보호(Protected)되어 있어 접근이 까다로우므로 우리가 직접 하나 들고 있습니다.
	TArray<AActor*> AllTrackedActors;

};
