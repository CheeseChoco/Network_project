// Fill out your copyright notice in the Description page of Project Settings.

#include "NPRepGraphNode_Grid.h"
#include "NPGridSubsystem.h" 
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UNPRepGraphNode_Grid::UNPRepGraphNode_Grid()
{
	bRequiresPrepareForReplicationCall = true;
}

void UNPRepGraphNode_Grid::PrepareForReplication()
{
	Super::PrepareForReplication();

	UE_LOG(LogTemp, Warning, TEXT("[GridNode] 업데이트 도는 중... 현재 들고 있는 액터 수: %d"), DynamicReplicatedActors.Num());

	UWorld* World = GetWorld();
	if (!World) return;

	UNPGridSubsystem* GridSubsystem = World->GetGameInstance()->GetSubsystem<UNPGridSubsystem>();
	if (!GridSubsystem || GridSubsystem->SharedBuffer.TotalCells == 0) return;

	// [디버그 시각화 생산자(Producer) 로직]
	TArray<uint8>& BackBuffer = GridSubsystem->SharedBuffer.GetBackBuffer();
	FMemory::Memzero(BackBuffer.GetData(), BackBuffer.Num() * sizeof(uint8));

	for (int32 Idx = 0; Idx < DynamicReplicatedActors.Num(); ++Idx)
	{
		FActorRepListType Actor = DynamicReplicatedActors[Idx];
		if (!Actor) continue;

		FVector Location = Actor->GetActorLocation();
		int32 GridX = FMath::FloorToInt(Location.X / GridSubsystem->SharedBuffer.CellSize);
		int32 GridY = FMath::FloorToInt(Location.Y / GridSubsystem->SharedBuffer.CellSize);

		int32 BufferIndex = GridSubsystem->SharedBuffer.GetIndex(GridX, GridY);
		if (BufferIndex != INDEX_NONE)
		{
			// 액터 1개당 강도 50씩 팍팍 올림! (눈에 확 띄게)
			BackBuffer[BufferIndex] = FMath::Min(255, BackBuffer[BufferIndex] + 50);
			UE_LOG(LogTemp, Warning, TEXT("Grid [%d, %d] Intensity: %d"), GridX, GridY, BackBuffer[BufferIndex]);
		}
	}

	// 1프레임에 1번만 안전하게 스왑!
	GridSubsystem->SharedBuffer.SwapBuffers();
}

void UNPRepGraphNode_Grid::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	Params.OutGatheredReplicationLists.AddReplicationActorList(DynamicReplicatedActors);
}


// =========================================================================
// [추가된 부분] 액터 등록 / 해제 로직
// =========================================================================

void UNPRepGraphNode_Grid::NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo)
{
	// 액터가 스폰되어 네트워크에 추가되면 우리 관리 리스트에 넣습니다.
	if (ActorInfo.Actor)
	{
		DynamicReplicatedActors.Add(ActorInfo.Actor);
	}
}

bool UNPRepGraphNode_Grid::NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound)
{
	// 액터가 파괴되거나 네트워크에서 제거되면 리스트에서 아주 빠르게 빼줍니다. (RemoveFast 사용)
	if (ActorInfo.Actor)
	{
		return DynamicReplicatedActors.RemoveFast(ActorInfo.Actor);
	}
	return false;
}

void UNPRepGraphNode_Grid::NotifyResetAllNetworkActors()
{
	// 게임 재시작이나 맵 이동 등으로 초기화가 필요할 때 리스트를 완전히 비웁니다.
	DynamicReplicatedActors.Reset();
}