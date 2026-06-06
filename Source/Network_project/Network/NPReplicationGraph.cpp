// Fill out your copyright notice in the Description page of Project Settings.

#include "NPReplicationGraph.h"
#include "Objects/EnemyBase.h"
#include "NPRepGraphNode_Grid.h"



UNPReplicationGraph::UNPReplicationGraph()
{

}



//액터별 세팅
void UNPReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();


	// 메모리에 올라와 있는 모든 클래스(UClass)를 순회합니다.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		// 해당 클래스의 붕어빵 틀(Class Default Object)을 가져옵니다.
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());

		// 1. CDO가 없거나, 리플리케이션(네트워크 동기화)이 꺼진 액터라면 장부에 적을 필요가 없습니다.
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}

		// 2. 블루프린트 컴파일 시 생성되는 임시 쓰레기 클래스(SKEL, REINST)들은 무시합니다.
		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		FClassReplicationInfo ClassInfo;

		// 3. 통신 주기 설정 (초당 업데이트 횟수를 서버 프레임 주기로 변환)
		ClassInfo.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(ActorCDO->GetNetUpdateFrequency());

		// 4. 가시거리(Cull Distance) 자동 설정
		if (ActorCDO->bAlwaysRelevant || ActorCDO->bOnlyRelevantToOwner)
		{
			// 글로벌이거나 개인용 액터는 거리 제한 없이 보내야 하므로 0으로 설정
			ClassInfo.SetCullDistanceSquared(0.f);
		}
		else
		{
			// 일반 액터는 블루프린트 디테일 패널에 적힌 Net Cull Distance 값을 그대로 사용
			ClassInfo.SetCullDistanceSquared(ActorCDO->GetNetCullDistanceSquared());

			if (Class->IsChildOf(ANPCharacter::StaticClass()))
			{
				float ActualCullDistSq = ActorCDO->GetNetCullDistanceSquared();
				// 클래스 이름, 제곱된 원본 값, 그리고 알기 쉽게 계산된 실제 미터(m) 거리를 출력합니다.
				UE_LOG(LogTemp, Warning, TEXT("[RepGraph] Class Loaded: %s"), *Class->GetName());
				UE_LOG(LogTemp, Warning, TEXT("  -> CDO NetCullDistanceSquared: %f"), ActualCullDistSq);
				UE_LOG(LogTemp, Warning, TEXT("  -> Actual In-Game Distance: ~%.1f meters"), FMath::Sqrt(ActualCullDistSq) / 100.0f);
				UE_LOG(LogTemp, Warning, TEXT("--------------------------------------------------"));
			}
		}

		// 5. 최종적으로 우리의 전역 장부(Map)에 등록!
		GlobalActorReplicationInfoMap.SetClassInfo(Class, ClassInfo);
	}
}

//그래프 생성 시 세팅
void UNPReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	GridNode = CreateNewNode<UNPRepGraphNode_Grid>();
	GridNode->CellSize = 1000.0f;
	GridNode->SpatialBias = FVector2D(-15000.0f, -15000.0f);
	AddGlobalGraphNode(GridNode);

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

//각 클라이언트에게 노드 만들기
void UNPReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
	Super::InitConnectionGraphNodes(RepGraphConnection);

	// 1. 새 접속자를 위한 개인용 노드(상자) 생성
	UReplicationGraphNode_AlwaysRelevant_ForConnection* ConnectionNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
	AddConnectionGraphNode(ConnectionNode, RepGraphConnection);

	// 2. [핵심] 장부(Array)에 '접속자 신분증(Connection)'과 '전용 상자(Node)'를 묶어서(Pair) 등록!
	AlwaysRelevantForConnectionList.Emplace(RepGraphConnection->NetConnection, ConnectionNode);
}

//액터 노드에 추가하기
void UNPReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	AActor* Actor = ActorInfo.GetActor();
	if (!Actor) return;

	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
	}
	else if (ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		ActorsWithoutNetConnection.Add(ActorInfo.Actor);
	}
	else
	{
		// Note that UReplicationGraphNode_GridSpatialization2D has 3 methods for adding actor based on the mobility of the actor. Since AActor lacks this information, we will
		// add all spatialized actors as dormant actors: meaning they will be treated as possibly dynamic (moving) when not dormant, and as static (not moving) when dormant.
		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
	}
}
//액터 노드에서 지우기
void UNPReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	Super::RouteRemoveNetworkActorToNodes(ActorInfo);

	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
		SetActorDestructionInfoToIgnoreDistanceCulling(ActorInfo.GetActor());
	}
	else if (ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		if (UReplicationGraphNode* Node = ActorInfo.Actor->GetNetConnection() ? GetAlwaysRelevantNodeForConnection(ActorInfo.Actor->GetNetConnection()) : nullptr)
		{
			Node->NotifyRemoveNetworkActor(ActorInfo);
		}
	}
	else
	{
		GridNode->RemoveActor_Dormancy(ActorInfo);
	}
}

UReplicationGraphNode_AlwaysRelevant_ForConnection* UNPReplicationGraph::GetAlwaysRelevantNodeForConnection(UNetConnection* Connection)
{
	UReplicationGraphNode_AlwaysRelevant_ForConnection* Node = nullptr;
	if (Connection)
	{
		if (FNPConnectionAlwaysRelevantNodePair* Pair = AlwaysRelevantForConnectionList.FindByKey(Connection))
		{
			if (Pair->Node)
			{
				Node = Pair->Node;
			}
			else
			{
				UE_LOG(LogNet, Warning, TEXT("AlwaysRelevantNode for connection %s is null."), *GetNameSafe(Connection));
			}
		}
		else
		{
			UE_LOG(LogNet, Warning, TEXT("Could not find AlwaysRelevantNode for connection %s. This should have been created in UBasicReplicationGraph::InitConnectionGraphNodes."), *GetNameSafe(Connection));
		}
	}
	else
	{
		// Basic implementation requires owner is set on spawn that never changes. A more robust graph would have methods or ways of listening for owner to change
		UE_LOG(LogNet, Warning, TEXT("Actor: bOnlyRelevantToOwner is set but does not have an owning Netconnection. It will not be replicated"));
	}

	return Node;
}

//Without으로 제외된 액터를 후에 설정
int32 UNPReplicationGraph::ServerReplicateActors(float DeltaSeconds)
{
	// Route Actors needing owning net connections to appropriate nodes
	for (int32 idx = ActorsWithoutNetConnection.Num() - 1; idx >= 0; --idx)
	{
		bool bRemove = false;
		if (AActor* Actor = ActorsWithoutNetConnection[idx])
		{
			if (UNetConnection* Connection = Actor->GetNetConnection())
			{
				bRemove = true;
				if (UReplicationGraphNode_AlwaysRelevant_ForConnection* Node = GetAlwaysRelevantNodeForConnection(Actor->GetNetConnection()))
				{
					Node->NotifyAddNetworkActor(FNewReplicatedActorInfo(Actor));
				}
			}
		}
		else
		{
			bRemove = true;
		}

		if (bRemove)
		{
			ActorsWithoutNetConnection.RemoveAtSwap(idx, EAllowShrinking::No);
		}
	}


	return Super::ServerReplicateActors(DeltaSeconds);
}

//서버 비교
bool FNPConnectionAlwaysRelevantNodePair::operator==(UNetConnection* InConnection) const
{
	// Any children should be looking at their parent connections instead.
	if (InConnection->GetUChildConnection() != nullptr)
	{
		InConnection = ((UChildConnection*)InConnection)->Parent;
	}

	return InConnection == NetConnection;
}

