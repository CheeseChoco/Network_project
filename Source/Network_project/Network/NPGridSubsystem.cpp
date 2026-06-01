// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NPGridSubsystem.h"

void UNPGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("NPGridSubsystem Initialized!"));
}

void UNPGridSubsystem::Deinitialize()
{
	// TArray는 자동으로 메모리가 해제되므로 별도의 처리가 필요 없습니다.
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("NPGridSubsystem Deinitialized."));
}

void UNPGridSubsystem::InitGridSystem(float WorldWidth, float WorldHeight, float CellSize)
{
	// 배열 메모리 실제 할당
	SharedBuffer.Init(WorldWidth, WorldHeight, CellSize);

	UE_LOG(LogTemp, Warning, TEXT("Grid System Setup Complete: %d x %d (Total Cells: %d, CellSize: %f)"),
		SharedBuffer.GridWidth, SharedBuffer.GridHeight, SharedBuffer.TotalCells, CellSize);
}