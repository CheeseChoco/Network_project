// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "NPReplicationGraph.generated.h"

class UNPRepGraphNode_Grid;

/**
 * 
 */
UCLASS()
class NETWORK_PROJECT_API UNPReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()
	
public:
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;

	// 우리가 만든 커스텀 그리드 노드 포인터
	UPROPERTY()
	UNPRepGraphNode_Grid* GridNode;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
};
