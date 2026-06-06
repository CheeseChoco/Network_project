// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "NPReplicationGraph.generated.h"

class UNPRepGraphNode_Grid;


USTRUCT()
struct FNPConnectionAlwaysRelevantNodePair
{
	GENERATED_BODY()
	FNPConnectionAlwaysRelevantNodePair() {}
	FNPConnectionAlwaysRelevantNodePair(UNetConnection* InConnection, UReplicationGraphNode_AlwaysRelevant_ForConnection* InNode) : NetConnection(InConnection), Node(InNode) {}
	bool operator==(UNetConnection* InConnection) const;

	UPROPERTY()
	TObjectPtr<UNetConnection> NetConnection = nullptr;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_AlwaysRelevant_ForConnection> Node = nullptr;
};



UCLASS()
class NETWORK_PROJECT_API UNPReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()
	
public:
	UNPReplicationGraph();


	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	virtual int32 ServerReplicateActors(float DeltaSeconds) override;

	UPROPERTY()
	TObjectPtr<UNPRepGraphNode_Grid> GridNode;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	UPROPERTY()
	TArray<FNPConnectionAlwaysRelevantNodePair> AlwaysRelevantForConnectionList;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsWithoutNetConnection;

	UReplicationGraphNode_AlwaysRelevant_ForConnection* GetAlwaysRelevantNodeForConnection(UNetConnection* Connection);
};
