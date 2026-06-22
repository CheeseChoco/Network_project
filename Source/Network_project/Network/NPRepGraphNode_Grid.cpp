// Fill out your copyright notice in the Description page of Project Settings.

#include "NPRepGraphNode_Grid.h"
#include "NPGridSubsystem.h" 
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UNPRepGraphNode_Grid::UNPRepGraphNode_Grid()
{
	// 더 이상 PrepareForReplication(매 프레임 무조건 도는 루프)를 강제로 켤 필요가 없습니다.
	bRequiresPrepareForReplicationCall = true;
}

void UNPRepGraphNode_Grid::NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo)
{

	// 2. 비주얼라이저 대조용으로 우리 명부에도 이름을 적어둡니다.
	/*if (ActorInfo.Actor)
	{
		AllTrackedActors.AddUnique(ActorInfo.Actor);
	}*/
}

bool UNPRepGraphNode_Grid::NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound)
{
	bool bResult = Super::NotifyRemoveNetworkActor(ActorInfo, bWarnIfNotFound);
	/*if (ActorInfo.Actor)
	{
		AllTrackedActors.Remove(ActorInfo.Actor);
	}*/
	return bResult;
}

void UNPRepGraphNode_Grid::NotifyResetAllNetworkActors()
{
	Super::NotifyResetAllNetworkActors();
	//AllTrackedActors.Reset();
}


//GatherActorListsForConnection에서 쓸 인라인 함수(ReplicationGraph.cpp의 함수를 가져온 것)
FORCEINLINE uint32 CalcDynamicReplicationPeriod(const float FinalPCT, const uint32 MinRepPeriod, const uint32 MaxRepPeriod, uint16& OutReplicationPeriodFrame, uint32& OutNextReplicationFrame, const uint32 LastRepFrameNum, const uint32 FrameNum, bool ForFastPath)
{
	const float PeriodRange = (float)(MaxRepPeriod - MinRepPeriod);
	const uint32 ExtraPeriod = (uint32)FMath::CeilToInt(PeriodRange * FinalPCT);

	const uint32 FinalPeriod = MinRepPeriod + ExtraPeriod;
	OutReplicationPeriodFrame = (uint16)FMath::Clamp<uint32>(FinalPeriod, 1, MAX_uint16);

	const uint32 NextRepFrameNum = LastRepFrameNum + OutReplicationPeriodFrame;
	OutNextReplicationFrame = NextRepFrameNum;


	return ExtraPeriod;
}


// 각 클라이언트의 프레임 마다 실행되는 최적화용 함수
void UNPRepGraphNode_Grid::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	Super::GatherActorListsForConnection(Params);

	repCheck(GraphGlobals.IsValid());

	if (Params.Viewers.Num() == 0 || Params.OutGatheredReplicationLists.NumLists() == 0) return;

	UReplicationGraph* RepGraph = GraphGlobals->ReplicationGraph;
	repCheck(RepGraph);
	repCheck(GraphGlobals->GlobalActorReplicationInfoMap);

	FGlobalActorReplicationInfoMap& GlobalMap = *GraphGlobals->GlobalActorReplicationInfoMap;
	FPerConnectionActorInfoMap& ConnectionActorInfoMap = Params.ConnectionManager.ActorInfoMap;
	const uint32 FrameNum = Params.ReplicationFrameNum;

	// 논리적 뷰포트 (이전 코드 동일 적용)
	FVector LogicalCenter = Params.Viewers[0].ViewLocation;
	if (AActor* ViewTarget = Params.Viewers[0].ViewTarget)
	{
		if (ViewTarget->IsA<APawn>()) LogicalCenter = ViewTarget->GetActorLocation();
		else if (UNetConnection* Connection = Params.ConnectionManager.NetConnection)
		{
			if (APlayerController* PC = Connection->GetPlayerController(GetWorld()))
				if (APawn* Pawn = PC->GetPawn()) LogicalCenter = Pawn->GetActorLocation();
		}
	}

	const float ViewExtentX = 900.f;
	const float ViewExtentY = 1600.f;
	FBox2D LogicalViewportAABB(
		FVector2D(LogicalCenter.X - ViewExtentX, LogicalCenter.Y - ViewExtentY),
		FVector2D(LogicalCenter.X + ViewExtentX, LogicalCenter.Y + ViewExtentY)
	);

	//UReplicationGraphNode_DynamicSpatialFrequency의 TwoPass 따라하기
	const int32 MaxNearestActors = 200;

	// 2. 수집 및 주기 판정 후 정렬 배열 초기화로 변경
	SortedReplicationList.Reset();
	NumExpectedReplicationsThisFrame = 0;
	NumExpectedReplicationsNextFrame = 0;


	for (AActor* Actor : Params.OutGatheredReplicationLists.ViewActors(EActorRepListTypeFlags::Default))
	{
		if (!Actor) continue;
		FGlobalActorReplicationInfo& GlobalInfo = GlobalMap.Get(Actor);
		float DistSq = FVector::DistSquaredXY(LogicalCenter, Actor->GetActorLocation());

		// 일단 거리(DistSq)를 넣어서 1차 배열 구성
		SortedReplicationList.Emplace(Actor, (int32)DistSq, &GlobalInfo);
	}

	if (SortedReplicationList.Num() > MaxNearestActors)
	{
		SortedReplicationList.Sort(); // 가까운 순으로 정렬
		SortedReplicationList.SetNum(MaxNearestActors, EAllowShrinking::No); // 꼬리 자르기
	}


	// Pass 2: Calc Frequency
	for (int32 idx = SortedReplicationList.Num() - 1; idx >= 0; --idx)
	{
		FNPSortedActorItem& Item = SortedReplicationList[idx];

		FConnectionReplicationActorInfo& ConnectionInfo = ConnectionActorInfoMap.FindOrAdd(Item.Actor);
		Item.ConnectionInfo = &ConnectionInfo;



		float FinalPCT = 1.0f;
		FVector ActorLoc = Item.Actor->GetActorLocation();
		if (LogicalViewportAABB.IsInside(FVector2D(ActorLoc.X, ActorLoc.Y))) FinalPCT = 0.0f;
		//else if (Item.FramesTillReplicate <= FMath::Square(1500.f)) FinalPCT = 0.5f;

		uint16 OutPeriod = 0; uint32 OutNextFrame = 0;
		CalcDynamicReplicationPeriod(FinalPCT, 1, 15, ConnectionInfo.ReplicationPeriodFrame, ConnectionInfo.NextReplicationFrameNum, ConnectionInfo.LastRepFrameNum, FrameNum, false);
		

		Item.FramesTillReplicate = (int32)ConnectionInfo.NextReplicationFrameNum - (int32)FrameNum;


		RepGraph->UpdateActorChannelCloseFrameNum(Item.Actor, ConnectionInfo, *Item.GlobalInfo, FrameNum, Params.ConnectionManager.NetConnection);


	}

	SortedReplicationList.Sort();
	Params.OutGatheredReplicationLists.Reset();

	// =====================================================================
// [Phase 4] 대역폭 캡 및 직접 전송 (API Replicate)
// =====================================================================
	const int64 MaxBitsThisFrame = 40000;
	int64 BitsWritten = 0;

	for (FNPSortedActorItem& Item : SortedReplicationList)
	{
		// 우선순위 정렬 덕분에, 전송 대상이 아닌 액터가 나오면 그 뒤는 안 봐도 됨
		if (Item.FramesTillReplicate > 0) break;
		if (BitsWritten > MaxBitsThisFrame) break;

		// [발사] 직접 직렬화 및 전송
		BitsWritten += RepGraph->ReplicateSingleActor(
			Item.Actor, *Item.ConnectionInfo, *Item.GlobalInfo,
			ConnectionActorInfoMap, Params.ConnectionManager, FrameNum
		);

		// [안전한 갱신] 실제 패킷 전송에 성공했으므로, 여기서 LastRepFrameNum을 갱신합니다.
		Item.ConnectionInfo->LastRepFrameNum = FrameNum;
	}



}

