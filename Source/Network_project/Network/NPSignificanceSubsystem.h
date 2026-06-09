//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Subsystems/WorldSubsystem.h"
//#include "NPSignificanceTypes.h"
//#include "NPSignificanceSubsystem.generated.h"
//
///**
// * 
// */
//UCLASS()
//class NETWORK_PROJECT_API UNPSignificanceSubsystem : public UTickableWorldSubsystem
//{
//	GENERATED_BODY()
//	
//public:
//	// 서브시스템 틱 오버라이드
//	virtual void Tick(float DeltaTime) override;
//	virtual TStatId GetStatId() const override { return TStatId(); }
//
//	// 액터가 스폰될 때 자신을 매니저에게 등록
//	void RegisterActor(AActor* InActor);
//	// 액터가 파괴될 때 장부에서 제거
//	void UnregisterActor(AActor* InActor);
//
//private:
//	// CPU 캐시 효율을 극대화하기 위한 연속된 구조체 배열 (장부)
//	UPROPERTY()
//	TArray<FNPActorSignificancePayload> ManagedActors;
//
//	// 부하 분산을 위한 인덱스 기억 장치
//	int32 CurrentEvaluationIndex = 0;
//
//	// 한 프레임에 최대 몇 개씩 검사할 것인가? (라이라 타임 슬라이싱 기법)
//	const int32 MaxEvaluationsPerFrame = 50;
//
//};
