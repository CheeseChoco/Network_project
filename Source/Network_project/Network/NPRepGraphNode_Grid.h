// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "NPRepGraphNode_Grid.generated.h"

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API UNPRepGraphNode_Grid : public UReplicationGraphNode
{
	GENERATED_BODY()
	
public:
	UNPRepGraphNode_Grid();

	// 리플리케이션 업데이트 루프의 핵심 함수
	virtual void PrepareForReplication() override;
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound = true) override;
	virtual void NotifyResetAllNetworkActors() override;


protected:
	// 이 노드에서 관리할 동적 액터들의 리스트 (기본적인 1D 배열 형태)
	// 실제 공간 분할 최적화를 하려면 2D 배열 형태의 컨테이너를 써야 하지만, 
	// 여기서는 시각화 로직에 집중하기 위해 기본 리스트를 순회한다고 가정합니다.
	FActorRepListRefView DynamicReplicatedActors;

};
