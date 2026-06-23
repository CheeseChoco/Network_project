// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

#include "Components/InterpToMovementComponent.h"

AEnemyBase::AEnemyBase()
{
    // [팀 규정] 적군은 태어날 때부터 2팀으로 규정합니다.
    TeamID = 2;

}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    checkf(AbilitySystemComponent, TEXT("ASC Null"));
    if (AbilitySystemComponent)
    {
        // 1. 태그 가져오기 (매번 FName으로 찾지 말고 캐싱하는 게 좋지만, 일단 직관적으로 씁니다)
        FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Character.Type.Enemy"));

        // 2. ASC에 태그 부착 (이러면 GAS 시스템 전체가 얘를 적으로 인식함)
        AbilitySystemComponent->AddLooseGameplayTag(EnemyTag);
    }

    // ====================================================================
    // [네트워크 최적화 시각화] 클라이언트 보간(Extrapolation) 원천 차단
    // ====================================================================

    // 1. 현재 액터에 부착된 InterpToMovement 컴포넌트를 동적으로 찾습니다.
    if (UInterpToMovementComponent* InterpComp = FindComponentByClass<UInterpToMovementComponent>())
    {
        // 2. 서버가 아닌 클라이언트(권한 없음)인 경우에만 진입합니다.
        if (!HasAuthority())
        {
            // 컴포넌트의 작동을 완전히 정지시키고, 매 프레임 도는 Tick 연산을 꺼버립니다.
            // 이로 인해 클라이언트는 미래 궤적을 짐작하지 못하고, 오직 서버 패킷에 의존해 강제 순간이동만 하게 됩니다.
            InterpComp->Deactivate();
            InterpComp->SetComponentTickEnabled(false);

            UE_LOG(LogTemp, Log, TEXT("[%s] Client: InterpToMovement Disabled for Replication Viz"), *GetName());
        }
    }
}
