// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

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
}
