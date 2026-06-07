// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/Atomic.h"
#include "NPGridSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FActorCullInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Visualizer")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Visualizer")
	float CullDistance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Visualizer")
	bool bIsReplicated = false;

	UPROPERTY(BlueprintReadOnly, Category = "Visualizer")
	FString ActorName;
};



struct FGridDoubleBuffer {
	float CellSize = 1000.0f; // 한 칸의 크기
	int32 GridWidth = 0;      // 가로 격자 개수
	int32 GridHeight = 0;     // 세로 격자 개수
	int32 TotalCells = 0;     // 총 격자 개수

	// 더블 버퍼링 용도 (고정 크기 타일 배열에서 가변 액터 정보 배열로 변경)
	TArray<FActorCullInfo> Buffer0;
	TArray<FActorCullInfo> Buffer1;

	// 아토믹 버퍼 스왑 인덱스
	TAtomic<int32> ActiveBufferIndex{ 0 };

	// 그리드 크기 및 기본 변수 초기화
	void Init(float InWorldWidth, float InWorldHeight, float InCellSize)
	{
		CellSize = InCellSize;
		GridWidth = FMath::CeilToInt(InWorldWidth / CellSize);
		GridHeight = FMath::CeilToInt(InWorldHeight / CellSize);
		TotalCells = GridWidth * GridHeight;

		Buffer0.Empty();
		Buffer1.Empty();
	}

	// 기록용 뒷 버퍼 호출
	TArray<FActorCullInfo>& GetBackBuffer()
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

	// 읽기용 앞 버퍼 호출
	const TArray<FActorCullInfo>& GetFrontBuffer() const
	{
		return (ActiveBufferIndex.Load(EMemoryOrder::SequentiallyConsistent) == 0) ? Buffer0 : Buffer1;
	}
};


UCLASS()
class NETWORK_PROJECT_API UNPGridSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	void InitGridSystem(float WorldWidth, float WorldHeight, float CellSize = 3000.0f);

	// -------------------------------------------------------------------------
	// 외부(게임모드나 리플리케이션 그래프)에서 호출하여 그리드를 최초 세팅하는 함수
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Visualizer")
	TArray<FActorCullInfo> GetCurrentCullInfoList() const;

	// 위에서 정의한 더블 버퍼 구조체의 실제 인스턴스
	FGridDoubleBuffer SharedBuffer;

};
