//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "NPSignificanceSubsystem.h"
//#include "GameFramework/PlayerController.h"
//
//void UNPSignificanceSubsystem::RegisterActor(AActor* InActor)
//{
//	if (!InActor || !InActor->Implements<UNPSignificantActor>()) return;
//
//	FNPActorSignificancePayload NewPayload;
//	NewPayload.ActorPtr = InActor;
//	// 등록할 때 단 한 번만 가상 함수를 호출하여 기본 점수를 캐싱해 둡니다. (오버헤드 방지)
//	NewPayload.BaseTierScore = Cast<INPSignificantActor>(InActor)->GetBaseTierScore();
//
//	ManagedActors.Add(NewPayload);
//}
//
//void UNPSignificanceSubsystem::UnregisterActor(AActor* InActor)
//{
//	// SwapMemory 기법으로 중간 배열을 삭제할 때 발생하는 메모리 시프트를 막습니다.
//	ManagedActors.RemoveAllSwap([InActor](const FNPActorSignificancePayload& Payload) {
//		return Payload.ActorPtr == InActor;
//		});
//}
//
//void UNPSignificanceSubsystem::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//	if (ManagedActors.IsEmpty()) return;
//
//	APlayerController* PC = GetWorld()->GetFirstPlayerController();
//	if (!PC || !PC->GetPawn()) return;
//
//	FVector CameraLoc = PC->GetPawn()->GetActorLocation();
//
//	// =========================================================
//	// [핵심] Time-slicing (타임 슬라이싱)
//	// 매 프레임 ManagedActors를 다 돌지 않고, 지정된 개수만큼만 쪼개서 검사합니다.
//	// =========================================================
//	int32 EvalsThisFrame = 0;
//
//	while (EvalsThisFrame < MaxEvaluationsPerFrame)
//	{
//		// 인덱스가 끝에 도달하면 다시 0으로 돌립니다.
//		if (CurrentEvaluationIndex >= ManagedActors.Num())
//		{
//			CurrentEvaluationIndex = 0;
//			break; // 이번 프레임의 사이클이 끝났으므로 다음 프레임에 재개
//		}
//
//		FNPActorSignificancePayload& Payload = ManagedActors[CurrentEvaluationIndex];
//		if (Payload.ActorPtr)
//		{
//			INPSignificantActor* SigInterface = Cast<INPSignificantActor>(Payload.ActorPtr);
//
//			// 1. 거리 계산 (2D 탑다운 기준 Z 무시)
//			float Dist2D = FVector::Dist2D(CameraLoc, Payload.ActorPtr->GetActorLocation());
//
//			// 2. 2D 우선도 산정 공식 
//			// (기본 등급 점수 + 전투 중이면 0.3 보너스) * 거리 감쇠율
//			float CombatBonus = SigInterface->IsInCombatState() ? 0.3f : 0.0f;
//			float DistanceMultiplier = 1.0f;
//
//			if (Dist2D > 2000.0f) DistanceMultiplier = 0.2f; // 완전 멂
//			else if (Dist2D > 1000.0f) DistanceMultiplier = 0.6f; // 조금 멂
//
//			float FinalScore = (Payload.BaseTierScore + CombatBonus) * DistanceMultiplier;
//
//			// 3. 버킷 판정 및 하달 (명령)
//			ENPSignificanceBucket TargetBucket;
//			if (FinalScore >= 0.8f) TargetBucket = ENPSignificanceBucket::Highest;
//			else if (FinalScore >= 0.5f) TargetBucket = ENPSignificanceBucket::High;
//			else if (FinalScore >= 0.2f) TargetBucket = ENPSignificanceBucket::Medium;
//			else TargetBucket = ENPSignificanceBucket::Low;
//
//			// 액터에게 "이 버킷 규칙대로 움직여라!" 하고 통보합니다.
//			SigInterface->ApplySignificanceBucket(TargetBucket);
//		}
//
//		CurrentEvaluationIndex++;
//		EvalsThisFrame++;
//	}
//}
