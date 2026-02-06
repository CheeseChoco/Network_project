// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"


APlayerCharacter::APlayerCharacter()
{
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->GroundFriction = 8.0f; // 높을수록 덜 미끄러짐
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f; // 카메라 거리
	CameraBoom->SetRelativeRotation(FRotator(-90.0f, 0.0f, 180.0f)); // 위에서 아래로 직각으로 내려다봄
	CameraBoom->bDoCollisionTest = false; // 카메라가 벽에 닿아도 줌인되지 않게

	// 6. 카메라 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true); // 멀티플레이 필수
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); // 플레이어는 Mixed가 국룰

	// AttributeSet 생성 (나중에 클래스 만들고 주석 해제)
	// AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));

	// ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// [중요] 스킬 부여 (GiveAbility)는 권한이 있는 서버에서만 수행해야 함!
	if (HasAuthority() && AbilitySystemComponent && SkillAbilityClass)
	{
		// 스킬을 캐릭터에게 "가르쳐줍니다" (장착)
		// GiveAbility는 SpecHandle을 반환하는데, 나중에 스킬 레벨업 등을 할 때 씁니다.
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SkillAbilityClass, 1, 0));
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (AbilitySystemComponent)
	{
		// GAS 초기화: 나(Owner)와 아바타(Avatar)가 누군지 알려줌
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// EnhancedInputComponent로 캐스팅하여 바인딩
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// MoveAction이 발동(Triggered)될 때 Move 함수 실행
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}
		if (SkillAction)
		{
			// Q키를 누르면(Started) ActivateSkill 함수 실행
			EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &APlayerCharacter::UseSkill);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FVector ForwardDirection = FVector(0, 1, 0); 
		const FVector RightDirection = FVector(1, 0, 0); 

		AddMovementInput(FVector(1, 0, 0), MovementVector.X);
		AddMovementInput(FVector(0, 1, 0), MovementVector.Y);
	}
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::UseSkill()
{
	if (AbilitySystemComponent && SkillAbilityClass)
	{
		// [핵심] 해당 클래스(GA_Fireball)를 찾아서 발동시킵니다.
		// TryActivateAbilityByClass는 내부적으로 알아서 네트워크 처리를 해줍니다.
		AbilitySystemComponent->TryActivateAbilityByClass(SkillAbilityClass);
	}
}