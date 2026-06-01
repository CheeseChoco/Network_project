// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/Atomic.h"
#include "NPGridSubsystem.generated.h"

/**
 * 
 */
struct FGridDoubleBuffer {
	float CellSize = 1000.0f; //한 칸의 크기(정사각형)
	int32 GridWidth = 0; //그리드의 가로 개수
	int32 GridHeight = 0; //그리드의 세로 개수
	int32 TotalCells = 0; //총 그리드 개수

	//더블 버퍼링 용도
	TArray<uint8> Buffer0;
	TArray<uint8> Buffer1;

	//아토믹
	TAtomic<int32> ActiveBufferIndex{ 0 };

	// 그리드 변수 초기화 용
	void Init(float InWorldWidth, float InWorldHeight, float InCellSize)
	{
		CellSize = InCellSize;

		//그리드 초기화
		GridWidth = FMath::CeilToInt(InWorldWidth / CellSize);
		GridHeight = FMath::CeilToInt(InWorldHeight / CellSize);
		TotalCells = GridWidth * GridHeight;

		// 버퍼 초기화
		Buffer0.SetNumZeroed(TotalCells);
		Buffer1.SetNumZeroed(TotalCells);
	}

	//2D 좌표 -> 그리드 좌표
	int32 GetIndex(int32 X, int32 Y) const
	{
		// 범위를 벗어나는 접근 차단
		if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight) return INDEX_NONE;
		return (Y * GridWidth) + X;
	}

	//기록용 뒷 버퍼 호출
	TArray<uint8>& GetBackBuffer()
	{
		return (ActiveBufferIndex.Load(EMemoryOrder::SequentiallyConsistent) == 0) ? Buffer1 : Buffer0;
	}

	// 기록 완료 후 앞배경/뒷배경 교체 (Atomic Swap)
	void SwapBuffers()
	{
		int32 CurrentActive = ActiveBufferIndex.Load(EMemoryOrder::SequentiallyConsistent);
		int32 NewActive = (CurrentActive == 0) ? 1 : 0;
		ActiveBufferIndex.Store(NewActive, EMemoryOrder::SequentiallyConsistent);
	}

	//읽기용 앞 버퍼 호출
	const TArray<uint8>& GetFrontBuffer() const
	{
		return (ActiveBufferIndex.Load(EMemoryOrder::SequentiallyConsistent) == 0) ? Buffer0 : Buffer1;
	}
};


UCLASS()
class NETWORK_PROJECT_API UNPGridSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 서브시스템 생성 시 자동으로 호출
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 서브시스템 소멸 시 자동으로 호출
	virtual void Deinitialize() override;

	// -------------------------------------------------------------------------
	// 외부(게임모드나 리플리케이션 그래프)에서 호출하여 그리드를 최초 세팅하는 함수
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void InitGridSystem(float WorldWidth, float WorldHeight, float CellSize = 3000.0f);

	// 위에서 정의한 더블 버퍼 구조체의 실제 인스턴스
	FGridDoubleBuffer SharedBuffer;

};
