// Fill out your copyright notice in the Description page of Project Settings.


#include "NPPlayerController.h"

ANPPlayerController::ANPPlayerController()
{
	// [핵심] 마우스 커서를 보이게 합니다.
	bShowMouseCursor = true;

	// (선택) 클릭 이벤트를 활성화합니다. (나중에 적 클릭할 때 필요)
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ANPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// [중요] 입력 모드 설정 (Game + UI)
	// 이걸 안 하면 빈 곳을 클릭했을 때 마우스가 다시 숨겨지거나 카메라가 휙 돌아갈 수 있습니다.
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스를 화면 밖으로 나가지 못하게 할지 여부
	InputModeData.SetHideCursorDuringCapture(false); // 클릭 중에도 커서 숨기지 않음

	SetInputMode(InputModeData);
}