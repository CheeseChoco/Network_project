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
		ConnectionInfo.LastRepFrameNum = FrameNum;

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









	//// =====================================================================
	//// [분기 B] 하드 캡 미만: 원 패스 (자를 필요 없으므로 바로 연산)
	//// =====================================================================
	//else
	//{
	//	for (const FActorRepListConstView& GatheredList : GatheredLists)
	//	{
	//		for (AActor* Actor : GatheredList)
	//		{
	//			if (!Actor) continue;
	//			FConnectionReplicationActorInfo* ConnectionInfo = ConnectionActorInfoMap.FindByObject(Actor);
	//			FGlobalActorReplicationInfo* GlobalInfo = GlobalMap.FindByObject(Actor);
	//			if (!ConnectionInfo || !GlobalInfo) continue;

	//			float DistSq = FVector::DistSquaredXY(LogicalCenter, Actor->GetActorLocation());
	//			if (DistSq > FMath::Square(3000.f)) continue;

	//			// 하드 캡 검사가 없으므로 넣기 전에 바로 계산
	//			float FinalPCT = 1.0f;
	//			FVector ActorLoc = Actor->GetActorLocation();
	//			if (LogicalViewportAABB.IsInside(FVector2D(ActorLoc.X, ActorLoc.Y))) FinalPCT = 0.0f;
	//			else if (DistSq <= FMath::Square(1500.f)) FinalPCT = 0.3f;

	//			uint16 OutPeriod = 0; uint32 OutNextFrame = 0;
	//			CalcDynamicReplicationPeriod(FinalPCT, 1, 15, OutPeriod, OutNextFrame, ConnectionInfo->LastReplicationFrameNum, CurrentFrameNum, false);
	//			ConnectionInfo->NextReplicationFrameNum = OutNextFrame;
	//			ConnectionInfo->LastReplicationFrameNum = CurrentFrameNum;

	//			SortedReplicationList.Add({ Actor, GlobalInfo, ConnectionInfo, DistSq });
	//		}
	//	}

	//	// 마지막 대역폭 분배를 위한 1회 정렬
	//	SortedReplicationList.Sort();
	//}

	//Params.OutGatheredReplicationLists.Reset();

	//for (const FActorRepListConstView& GatheredList : Params.OutGatheredReplicationLists.GetLists(EActorRepListTypeFlags::Default))
	//{
	//	for (AActor* Actor : GatheredList)
	//	{
	//		if (!Actor) continue;

	//		FConnectionReplicationActorInfo* ConnectionInfo = ConnectionActorInfoMap.FindByObject(Actor);
	//		FGlobalActorReplicationInfo* GlobalInfo = GlobalMap.FindByObject(Actor);

	//		if (!ConnectionInfo || !GlobalInfo) continue;

	//		// 대기 상태인 액터는 건너뛰기
	//		if (FrameNum < ConnectionInfo->NextReplicationFrameNum) continue;

	//		FVector ActorLoc = Actor->GetActorLocation();
	//		float DistSq = FVector::DistSquaredXY(LogicalCenter, ActorLoc);

	//		if (DistSq > FMath::Square(3000.f)) continue; // 하드 컬링

	//		float FinalPCT = 1.0f; // Tier 3
	//		if (LogicalViewportAABB.IsInside(FVector2D(ActorLoc.X, ActorLoc.Y))) FinalPCT = 0.0f; // Tier 1
	//		else if (DistSq <= FMath::Square(1500.f)) FinalPCT = 0.3f; // Tier 2

	//		uint16 OutPeriod = 0;
	//		uint32 OutNextFrame = 0;
	//		CalcDynamicReplicationPeriod(FinalPCT, 1, 15, OutPeriod, OutNextFrame, ConnectionInfo->LastReplicationFrameNum, CurrentFrameNum, false);


	//		// 정렬 리스트에 구조체 생성 후 추가
	//		SortedReplicationList.Add({ Actor, GlobalInfo, ConnectionInfo, DistSq });
	//	}
	//}

	//// 3. 거리 순으로 정렬 (가장 중요한/가까운 액터가 0번 인덱스로 오게 됨)
	//SortedReplicationList.Sort();

	//// [매우 중요] 4. 부모가 만들어준 기존 리스트 초기화
	//// 여기서 비워주지 않으면, 우리가 패킷을 쐈는데 엔진이 뒷단에서 또 패킷을 쏘는 이중 전송 버그가 발생합니다.
	//Params.OutGatheredReplicationLists.Reset();

	//// 5. 리플리케이션 직접 수행 및 대역폭 제어
	//const int64 MaxBitsThisFrame = 40000; // 예시: 프레임당 최대 40,000 Bits (약 5KB) 할당
	//int64 BitsWritten = 0;

	//for (const FNPSortedActorItem& Item : SortedReplicationList)
	//{
	//	// 대역폭 초과 시 더 이상 멀리 있는 액터들은 전송하지 않고 다음 프레임으로 넘김
	//	if (BitsWritten > MaxBitsThisFrame)
	//	{
	//		break;
	//	}

	//	// 엔진 코어 함수: 직접 패킷을 직렬화하여 클라이언트에게 쏨
	//	BitsWritten += RepGraph->ReplicateSingleActor(
	//		Item.Actor,
	//		*Item.ConnectionInfo,
	//		*Item.GlobalInfo,
	//		ConnectionActorInfoMap,
	//		Params.ConnectionManager,
	//		FrameNum
	//	);
	//}




















	////for문 대신 예외처리로 안전장치
	//if (Params.Viewers.Num() == 0)
	//{
	//	return;
	//}

	//const FVector ViewerLocation = Params.Viewers[0].ViewLocation;

	//const uint32 CurrentTick = Params.ConnectionManager.ConnectionOrderNum;

	//FActorRepListRefView FilteredList = Params.ConnectionManager.AllocateRepList();

	//// =====================================================================
	//// [3. 명단 순회 및 거리 기반 주파수 커팅]
	//// =====================================================================
	//// 부모가 작성한 명단이 비어있지 않다면 순회를 시작합니다.
	//if (Params.OutGatheredReplicationLists.NumLists() > 0)
	//{
	//	for (const FActorRepListConstView& GatheredList : Params.OutGatheredReplicationLists.GetLists(EActorRepListTypeFlags::Default))
	//	{
	//		for (AActor* Actor : GatheredList)
	//		{
	//			if (!Actor) continue;

	//			// 플레이어와 액터 사이의 거리 제곱 (루트 연산을 피하기 위한 Sqaured 사용)
	//			float DistSq = FVector::DistSquaredXY(ViewerLocation, Actor->GetActorLocation());
	//			bool bPassFilter = true;

	//			// [핵심] 티어(Tier)별 프레임 스킵 로직
	//			if (DistSq > FMath::Square(3000.f))
	//			{
	//				// Tier 3 (원거리): 15틱 중 1틱만 통과 (나머지 14틱은 bPassFilter가 false가 됨)
	//				if (CurrentTick % 15 != 0) bPassFilter = false;
	//			}
	//			else if (DistSq > FMath::Square(1500.f))
	//			{
	//				// Tier 2 (중거리): 5틱 중 1틱만 통과
	//				if (CurrentTick % 5 != 0) bPassFilter = false;
	//			}
	//			// Tier 1 (근거리): 1500 미만은 무조건 true를 유지하여 매 틱 전송

	//			// 필터를 통과한 '찐' 액터들만 우리의 새 장부에 적습니다.
	//			if (bPassFilter)
	//			{
	//				FilteredList.Add(Actor);
	//			}
	//		}
	//	}

	//	// =====================================================================
	//// [4. 원본 명단 덮어씌우기]
	//// =====================================================================
	//// 부모가 채워둔 원본 바구니를 싹 비우고, 우리의 엄격한 필터를 통과한 새 바구니로 교체합니다.
	//	Params.OutGatheredReplicationLists.Reset();

	//	if (FilteredList.Num() > 0)
	//	{
	//		Params.OutGatheredReplicationLists.AddList(FilteredList);
	//	}

	//	// (이후 과정인 GridSubsystem 시각화 버퍼 전송 로직은 이 아래에 이어서 작성하시면 됩니다.)
	//}





















	// =====================================================================
	// [2. 결과 스파이(Spy) 및 추출]
	// =====================================================================
	//UWorld* World = GetWorld();
	//if (!World) return;

	//UNPGridSubsystem* GridSubsystem = World->GetGameInstance()->GetSubsystem<UNPGridSubsystem>();
	//if (!GridSubsystem) return;

	//// 빠른 검색을 위해 '이번 프레임에 통과한 액터들'을 해시셋(TSet)으로 옮겨 담습니다.
	//TSet<AActor*> ReplicatedActorsThisFrame;

	//// OutGatheredReplicationLists 내부를 순회합니다. (언리얼 버전에 따라 GetLists() 방식이 쓰입니다)
	//// 이 리스트에 이름이 있다면, 엔진 2D 노드가 "이 액터는 플레이어에게 전송하라"고 허락한 것입니다.
	//if (Params.OutGatheredReplicationLists.NumLists() > 0)
	//{
	//	for (const FActorRepListConstView& GatheredList : Params.OutGatheredReplicationLists.GetLists(EActorRepListTypeFlags::Default))
	//	{
	//		for (AActor* Actor : GatheredList)
	//		{
	//			if (Actor) ReplicatedActorsThisFrame.Add(Actor);
	//		}
	//	}
	//}

	//// 3. 서브시스템의 뒷배경 버퍼를 열어서 싹 비웁니다.
	//TArray<FActorCullInfo>& BackBuffer = GridSubsystem->SharedBuffer.GetBackBuffer();
	//BackBuffer.Empty();

	//// 4. 맵에 있는 "전체 액터" 명부를 돌면서 성적표(CullInfo)를 작성합니다.
	//for (AActor* Actor : AllTrackedActors)
	//{
	//	if (!Actor) continue;

	//	FActorCullInfo CullInfo;
	//	CullInfo.ActorName = Actor->GetName();
	//	CullInfo.Location = Actor->GetActorLocation();

	//	// 이 액터가 요구하는 가시거리 반경 (기본 세팅값 기준)
	//	// 엔진의 CullDistanceSquared는 제곱값이므로 화면에 원을 그리기 위해 루트(Sqrt)를 씌워줍니다.
	//	CullInfo.CullDistance = FMath::Sqrt(Actor->GetNetCullDistanceSquared());

	//	// 핵심: 통과 명단(ReplicatedActorsThisFrame)에 내 이름이 있으면 True, 멀어서 짤렸으면 False!
	//	CullInfo.bIsReplicated = ReplicatedActorsThisFrame.Contains(Actor);

	//	// 서브시스템 버퍼에 밀어 넣습니다.
	//	BackBuffer.Add(CullInfo);
	//}

	// 5. 기록이 끝났으니 비주얼라이저가 그림을 그릴 수 있도록 버퍼를 교체(Swap)합니다.
	//GridSubsystem->SharedBuffer.SwapBuffers();
}

