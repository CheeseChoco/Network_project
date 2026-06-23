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

	// 1. 기준점: 현재 내 화면을 보고 있는 로컬 플레이어의 위치
	FVector LogicalCenter = LocalPC->GetPawn()->GetActorLocation();

	// 2. 서버 랩그래프와 완벽히 동일한 AABB 규격 생성 (앞뒤 900, 좌우 1600)
	const float ViewExtentX = 900.f;
	const float ViewExtentY = 1600.f;
	FBox2D LogicalViewportAABB(
		FVector2D(LogicalCenter.X - ViewExtentX, LogicalCenter.Y - ViewExtentY),
		FVector2D(LogicalCenter.X + ViewExtentX, LogicalCenter.Y + ViewExtentY)
	);

	// 3. 현재 몬스터의 2D 평면 위치
	FVector ActorLoc = GetActorLocation();
	FVector2D ActorLoc2D(ActorLoc.X, ActorLoc.Y);

	// =======================================================================
	// 4. AABB 내부 판정 (서버 노드의 IsInside 로직과 100% 동일)
	// =======================================================================
	if (LogicalViewportAABB.IsInside(ActorLoc2D))
	{
		// 박스 안 (서버 기준: 매 프레임 동기화 대상)
		StateName = TEXT("Active [Inside AABB]");
		DebugColor = FColor::Green;
	}
	else
	{
		// 박스 밖 (서버 기준: 15프레임 지연 대상)
		StateName = TEXT("Culled [Outside AABB]");
		DebugColor = FColor::Red;
	}

	// =======================================================================
	// 디버그 출력 (캐릭터 머리 위에 거리와 현재 최적화 상태 표시)
	// =======================================================================
	FVector TextLocation = GetActorLocation() + FVector(0.f, 0.f, 120.f); // 머리 위쪽으로 살짝 올림
	FString DebugText = FString::Printf(TEXT("Dist: %.0f\n%s"), Dist2D, *StateName);

	// 지속 시간을 타이머 주기(0.5초)와 똑같이 맞춰서 글자가 겹치거나 깜빡이지 않게 부드럽게 유지합니다.
	DrawDebugString(GetWorld(), TextLocation, DebugText, nullptr, DebugColor, 0.5f, false, 1.2f);
}