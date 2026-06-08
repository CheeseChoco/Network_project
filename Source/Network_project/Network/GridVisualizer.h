// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridVisualizer.generated.h"

UCLASS()
class NETWORK_PROJECT_API AGridVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	AGridVisualizer();

protected:
	virtual void BeginPlay() override;
	/*virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;*/

public:	
	// Called every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid System")
	float GridCellSize = 1000.0f;

	//// 디버그 선을 그릴 때 사용할 Z 높이 (탑다운 게임의 바닥 높이에 맞추면 좋습니다)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid System")
	//float DrawZHeight = 0.0f;

	//// 1. 핵심 변환 함수: 월드 좌표를 넣으면 격자의 인덱스(X, Y)를 반환합니다. (Lyra 방식)
	//UFUNCTION(BlueprintPure, Category = "Grid System")
	//FIntPoint WorldToGridIndex(const FVector& WorldLocation) const;

	//// 2. 역변환 함수: 격자 인덱스(X, Y)를 넣으면 해당 칸의 '정중앙 월드 좌표'를 반환합니다. (선 그릴 때 필수)
	//UFUNCTION(BlueprintPure, Category = "Grid System")
	//FVector GridIndexToWorldCenter(const FIntPoint& GridIndex) const;



private:
	void DrawCellGrid();


	FTimerHandle DrawTimerHandle;

	//Cell 시각화 마진
	float Margin = 10.0f;

};
