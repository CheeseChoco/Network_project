//#pragma once
//
//#include "CoreMinimal.h"
//#include "UObject/Interface.h"
//#include "NPSignificanceTypes.generated.h"
//
//// 1. 상태를 구분할 버킷(Bucket) 열거형
//UENUM(BlueprintType)
//enum class ENPSignificanceBucket : uint8
//{
//	Highest,	// 화면 안, 매 프레임 틱
//	High,		// 화면 근처, 0.2초 틱
//	Medium,		// 화면 밖, 1.0초 틱
//	Low			// 완전 멀어짐, 틱 정지 및 Dormancy
//};
//
//// 2. 매니저가 들고 있을 가벼운 장부 (데이터 중심 설계의 핵심)
//USTRUCT()
//struct FNPActorSignificancePayload
//{
//	GENERATED_BODY()
//
//	// 매니저가 통제할 액터의 포인터
//	UPROPERTY()
//	AActor* ActorPtr = nullptr;
//
//	// 매번 액터에게 물어보지 않기 위해 기본 가중치(Tier)를 캐싱해 둡니다.
//	float BaseTierScore = 0.0f;
//};
//
//// 3. 디커플링을 위한 인터페이스
//UINTERFACE(MinimalAPI)
//class UNPSignificanceTypes : public UInterface { GENERATED_BODY() };
//
//class INPSignificanceTypes
//{
//	GENERATED_BODY()
//
//public:
//	// 매니저가 "너 점수 몇 점짜리 개체야?" 하고 물어볼 때 반환 (예: 보스 1.0, 몹 0.5)
//	virtual float GetBaseTierScore() const = 0;
//
//	// 현재 전투 중인지(어그로 상태) 반환
//	virtual bool IsInCombatState() const = 0;
//
//	// 매니저가 최종 계산 후 "너 이 버킷으로 들어가서 최적화해!" 라고 명령을 내리는 콜백
//	virtual void ApplySignificanceBucket(ENPSignificanceBucket NewBucket) = 0;
//};