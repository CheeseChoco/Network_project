// Fill out your copyright notice in the Description page of Project Settings.

#include "NPReplicationGraph.h"
#include "NPRepGraphNode_Grid.h"

void UNPReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();
	// (액터 종류별로 어떻게 리플리케이션할지 세팅하는 곳)
}

void UNPReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// 1. 우리가 만든 커스텀 그리드 시각화 노드를 생성합니다.
	GridNode = CreateNewNode<UNPRepGraphNode_Grid>();

	// 2. 이 노드를 전역(Global) 그래프에 등록하여 매 프레임 동작하게 만듭니다.
	AddGlobalGraphNode(GridNode);
}

void UNPReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	// 기본 로직 실행
	Super::RouteAddNetworkActorToNodes(ActorInfo, GlobalInfo);
	if (ActorInfo.Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RepGraph] 새 액터 들어옴: %s"), *ActorInfo.Actor->GetName());
	}
	// 우리가 만든 그리드 노드에 새 액터 정보를 전달!
	if (GridNode)
	{
		GridNode->NotifyAddNetworkActor(ActorInfo);
	}
}

void UNPReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	Super::RouteRemoveNetworkActorToNodes(ActorInfo);

	if (GridNode)
	{
		GridNode->NotifyRemoveNetworkActor(ActorInfo);
	}
}