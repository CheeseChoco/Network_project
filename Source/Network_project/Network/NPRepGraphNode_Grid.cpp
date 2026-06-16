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

// 각 클라이언트의 프레임 마다 실행되는 최적화용 함수
void UNPRepGraphNode_Grid::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	// =====================================================================
	// [1. 엔진의 최적화 실행]
	// 부모 클래스의 진짜 연산을 먼저 실행합니다. 
	// 이 함수가 끝나면 플레이어와 거리가 가까워서 "통과(Replicated)된" 액터들만 
	// Params.OutGatheredReplicationLists 바구니에 담기게 됩니다.
	// =====================================================================
	Super::GatherActorListsForConnection(Params);

	//for문 대신 예외처리로 안전장치
	if (Params.Viewers.Num() == 0)
	{
		return;
	}

	const FVector ViewerLocation = Params.Viewers[0].ViewLocation;

	const uint32 CurrentTick = Params.ConnectionManager.ConnectionOrderNum;

	FActorRepListRefView FilteredList = Params.ConnectionManager.AllocateRepList();

	// =====================================================================
	// [3. 명단 순회 및 거리 기반 주파수 커팅]
	// =====================================================================
	// 부모가 작성한 명단이 비어있지 않다면 순회를 시작합니다.
	if (Params.OutGatheredReplicationLists.NumLists() > 0)
	{
		for (const FActorRepListConstView& GatheredList : Params.OutGatheredReplicationLists.GetLists(EActorRepListTypeFlags::Default))
		{
			for (AActor* Actor : GatheredList)
			{
				if (!Actor) continue;

				// 플레이어와 액터 사이의 거리 제곱 (루트 연산을 피하기 위한 Sqaured 사용)
				float DistSq = FVector::DistSquaredXY(ViewerLocation, Actor->GetActorLocation());
				bool bPassFilter = true;

				// [핵심] 티어(Tier)별 프레임 스킵 로직
				if (DistSq > FMath::Square(3000.f))
				{
					// Tier 3 (원거리): 15틱 중 1틱만 통과 (나머지 14틱은 bPassFilter가 false가 됨)
					if (CurrentTick % 15 != 0) bPassFilter = false;
				}
				else if (DistSq > FMath::Square(1500.f))
				{
					// Tier 2 (중거리): 5틱 중 1틱만 통과
					if (CurrentTick % 5 != 0) bPassFilter = false;
				}
				// Tier 1 (근거리): 1500 미만은 무조건 true를 유지하여 매 틱 전송

				// 필터를 통과한 '찐' 액터들만 우리의 새 장부에 적습니다.
				if (bPassFilter)
				{
					FilteredList.Add(Actor);
				}
			}
		}

		// =====================================================================
	// [4. 원본 명단 덮어씌우기]
	// =====================================================================
	// 부모가 채워둔 원본 바구니를 싹 비우고, 우리의 엄격한 필터를 통과한 새 바구니로 교체합니다.
		Params.OutGatheredReplicationLists.Reset();

		if (FilteredList.Num() > 0)
		{
			Params.OutGatheredReplicationLists.AddList(FilteredList);
		}

		// (이후 과정인 GridSubsystem 시각화 버퍼 전송 로직은 이 아래에 이어서 작성하시면 됩니다.)
	}





















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

