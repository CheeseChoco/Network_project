// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NPAttributeSet.h"

// Sets default values
ANPCharacter::ANPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 1. 전투 시스템 장착
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UNPAttributeSet>(TEXT("AttributeSet"));


	bReplicates = true;
	SetNetCullDistanceSquared(FMath::Square(5000.0f));
}

// Called when the game starts or when spawned
void ANPCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// GAS 초기화: 나(Owner)와 아바타(Avatar)가 누군지 알려줌
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (AttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
			.AddUObject(this, &ANPCharacter::OnHealthChanged);
	}

	if (!HasAuthority() && !IsLocallyControlled())
	{
		// 0.5초마다 거리를 재고 상태를 디버깅합니다.
		GetWorld()->GetTimerManager().SetTimer(OptimizationTimerHandle, this, &ANPCharacter::OptimizeClientPerformance, 0.5f, true);
	}
	
}

void ANPCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	float newHealth = Data.NewValue;
	float oldHealth = Data.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("Ouch!!!! HP changed from %f to %f"), oldHealth, newHealth);

	// 3. 체력이 0보다 컸는데, 방금 데미지를 입고 0 이하가 된 경우에만 Die 호출
	if (newHealth <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("has died."));
		Die();
	}
}

void ANPCharacter::Die()
{
	// 플레이어와 적군이 공통으로 가지는 사망 로직 (필요 시 작성)
	UE_LOG(LogTemp, Log, TEXT("%s has died."), *GetName());
}

UAbilitySystemComponent* ANPCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGenericTeamId ANPCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}


void ANPCharacter::OptimizeClientPerformance()
{
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->GetPawn()) return;

	// 내 캐릭터(카메라 기준점)와의 2D 평면 거리 계산
	float Dist2D = FVector::Dist2D(GetActorLocation(), LocalPC->GetPawn()->GetActorLocation());

	float NewTickInterval = 0.0f;
	FString StateName = TEXT("");
	FColor DebugColor = FColor::White;

	// 1. 활성화 구역 (10m 이내)
	if (Dist2D <= 1000.0f)
	{
		NewTickInterval = 0.0f;
		StateName = TEXT("Active [Tick: 0.0s]");
		DebugColor = FColor::Green;
	}
	// 2. 둔화 구역 (10m ~ 20m)
	else if (Dist2D <= 2000.0f)
	{
		NewTickInterval = 5.0f;
		StateName = TEXT("Slowed [Tick: 5.0s]");
		DebugColor = FColor::Yellow;
	}
	// 3. 완전 삭제 대기 구역 (20m 밖)
	else
	{
		NewTickInterval = 10.0f;
		StateName = TEXT("Dormant [Tick: 10.0s]");
		DebugColor = FColor::Red;
	}

	// 1. 액터 본체의 틱을 조절합니다.
	SetActorTickInterval(NewTickInterval);

	// =======================================================================
	// [확장성 해결] 특정 컴포넌트 이름 대신, 이 액터에 붙은 '모든 컴포넌트'를 가져옵니다.
	// =======================================================================
	TArray<UActorComponent*> AllComponents;
	GetComponents(AllComponents);

	for (UActorComponent* Comp : AllComponents)
	{
		// 틱을 사용하는 활성화된 컴포넌트라면 모조리 틱 주기를 액터와 동일하게 맞춰버립니다.
		if (Comp && Comp->PrimaryComponentTick.bCanEverTick)
		{
			Comp->SetComponentTickInterval(NewTickInterval);
		}
	}
	// =======================================================================
	// 디버그 출력 (캐릭터 머리 위에 거리와 현재 최적화 상태 표시)
	// =======================================================================
	FVector TextLocation = GetActorLocation() + FVector(0.f, 0.f, 120.f); // 머리 위쪽으로 살짝 올림
	FString DebugText = FString::Printf(TEXT("Dist: %.0f\n%s"), Dist2D, *StateName);

	// 지속 시간을 타이머 주기(0.5초)와 똑같이 맞춰서 글자가 겹치거나 깜빡이지 않게 부드럽게 유지합니다.
	DrawDebugString(GetWorld(), TextLocation, DebugText, nullptr, DebugColor, 0.5f, false, 1.2f);
}