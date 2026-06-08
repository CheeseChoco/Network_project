#include "GridVisualizer.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NPGridSubsystem.h" // 서브시스템 헤더 포함


AGridVisualizer::AGridVisualizer()
{
    PrimaryActorTick.bCanEverTick = false; // 퍼포먼스를 위해 Tick은 끕니다.
}

void AGridVisualizer::BeginPlay()
{
    Super::BeginPlay();

	// 1. 기존의 서브시스템 초기화 로직 (유지)
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UNPGridSubsystem* GridSubsystem = GameInstance->GetSubsystem<UNPGridSubsystem>())
		{
			// 서브시스템 버퍼 초기화
			GridSubsystem->InitGridSystem(10000.0f, 10000.0f, GridCellSize);
		}
	}

	// =========================================================================
	// 2. [수정됨] 랩그래프 격자 렌더링 타이머 등록
	// 클라이언트 화면에서도 보여야 하므로 if (HasAuthority()) 조건문을 제거했습니다.
	// =========================================================================
	if (GetWorld())
	{
		// 1초마다 DrawReplicationGrid 함수를 반복 실행합니다. 
		// (DrawDebugBox의 LifeTime을 1초로 두었기 때문에 깜빡임 없이 유지됩니다)
		GetWorld()->GetTimerManager().SetTimer(DrawTimerHandle, this, &AGridVisualizer::DrawCellGrid, 1.0f, true);
	}
}

void AGridVisualizer::DrawCellGrid()
{
	// 더 이상 GridNode를 체크하지 않습니다. GetWorld()만 확인합니다.
	if (!GetWorld()) return;

	// =========================================================================
	// [수정됨] 데이터 디커플링
	// 랩그래프 노드에서 가져오지 않고, Visualizer가 들고 있는 설정값이나 Subsystem을 사용합니다.
	// =========================================================================
	float CellSize = GridCellSize;

	// Bias는 엔진 기본 랩그래프 세팅인 -UE_OLD_WORLD_MAX로 고정 (또는 변수화)
	FVector2D Bias(-UE_OLD_WORLD_MAX, -UE_OLD_WORLD_MAX);

	FVector Extent((CellSize * 0.5f) - Margin, (CellSize * 0.5f) - Margin, 10.0f);

	// 시각화할 맵의 물리적 바운더리 (-1만 ~ +1만 영역)
	float MapMinX = -10000.0f;
	float MapMaxX = 10000.0f;
	float MapMinY = -10000.0f;
	float MapMaxY = 10000.0f;

	// 바운더리를 랩그래프 내부의 가상 인덱스로 변환
	int32 MinIndexX = FMath::FloorToInt((MapMinX - Bias.X) / CellSize);
	int32 MaxIndexX = FMath::FloorToInt((MapMaxX - Bias.X) / CellSize);
	int32 MinIndexY = FMath::FloorToInt((MapMinY - Bias.Y) / CellSize);
	int32 MaxIndexY = FMath::FloorToInt((MapMaxY - Bias.Y) / CellSize);

	for (int32 X = MinIndexX; X <= MaxIndexX; ++X)
	{
		for (int32 Y = MinIndexY; Y <= MaxIndexY; ++Y)
		{
			FVector Center;
			Center.X = (X * CellSize) + Bias.X + (CellSize * 0.5f);
			Center.Y = (Y * CellSize) + Bias.Y + (CellSize * 0.5f);
			Center.Z = 0.1;

			DrawDebugBox(
				GetWorld(),
				Center,
				Extent,
				FQuat::Identity,
				FColor::Black,
				false,
				1.0f,
				0,
				20.0f
			);
		}
	}
}














































// 과거에 쓰던 코드
// 
//AGridVisualizer::AGridVisualizer()
//{
//	PrimaryActorTick.bCanEverTick = true;
//	PrimaryActorTick.TickInterval = 0.1f;
//}

//
//void AGridVisualizer::BeginPlay()
//{
//	Super::BeginPlay();
//
//	if (UGameInstance* GameInstance = GetGameInstance())
//	{
//		if (UNPGridSubsystem* GridSubsystem = GameInstance->GetSubsystem<UNPGridSubsystem>())
//		{
//			// [수정됨] 거대한 2D 배열을 메모리에 할당하던 과거 로직 탈피.
//			// 이제 이 값(10000x10000)은 메모리 할당이 아니라, 오직 "바닥에 디버그 격자선을 그릴 때 사용할 범위"로만 쓰입니다.
//			// 버퍼 자체는 동적(Empty 상태)으로 안전하게 초기화됩니다.
//			GridSubsystem->InitGridSystem(10000.0f, 10000.0f, GridCellSize);
//		}
//	}
//
//	
//}
//
//bool AGridVisualizer::ShouldTickIfViewportsOnly() const
//{
//	return true;
//}
//
//FIntPoint AGridVisualizer::WorldToGridIndex(const FVector& WorldLocation) const
//{
//	int32 IndexX = FMath::FloorToInt(WorldLocation.X / GridCellSize);
//	int32 IndexY = FMath::FloorToInt(WorldLocation.Y / GridCellSize);
//	return FIntPoint(IndexX, IndexY);
//}
//
//FVector AGridVisualizer::GridIndexToWorldCenter(const FIntPoint& GridIndex) const
//{
//	float CenterX = (GridIndex.X * GridCellSize) + (GridCellSize * 0.5f);
//	float CenterY = (GridIndex.Y * GridCellSize) + (GridCellSize * 0.5f);
//	return FVector(CenterX, CenterY, DrawZHeight);
//}
//
//void AGridVisualizer::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//	UGameInstance* GameInstance = GetGameInstance();
//	if (!GameInstance) return;
//
//	UNPGridSubsystem* GridSubsystem = GameInstance->GetSubsystem<UNPGridSubsystem>();
//	if (!GridSubsystem) return;
//
//	// =========================================================
//	// 1. 서브시스템에서 안전하게 현재 프레임의 액터 상태 리스트 스냅샷 복사
//	// =========================================================
//	TArray<FActorCullInfo> CurrentActorStates = GridSubsystem->GetCurrentCullInfoList();
//
//	// =========================================================
//	// 2. 수집된 액터 상태들을 순회하며 디버그 렌더링
//	// =========================================================
//	for (const FActorCullInfo& Info : CurrentActorStates)
//	{
//		// [핵심] 통과 여부에 따른 색상 결정 (Replicated = 초록색, Culled = 빨간색)
//		FColor DrawColor = Info.bIsReplicated ? FColor::Green : FColor::Red;
//
//		// 3D 공간의 Z 높이 보정 (땅바닥에 붙여서 그리기 위함)
//		FVector CenterLocation = Info.Location;
//		CenterLocation.Z = DrawZHeight;
//
//		// 액터 위치에 가시거리(CullDistance) 크기의 원(Circle)을 그립니다.
//		DrawDebugCircle(
//			GetWorld(),
//			CenterLocation,
//			Info.CullDistance,      // 반지름 (데이터 테이블/전역 세팅에서 지정한 거리)
//			36,                     // 원을 구성하는 선분 개수 (클수록 부드러움)
//			DrawColor,              // 색상
//			false,                  // 유지 여부 (매 프레임 그리므로 false)
//			-1.0f,                  // 유지 시간
//			0,                      // 우선순위
//			5.0f,                   // 선 두께 (눈에 잘 띄게 5.0f)
//			FVector(0, 1, 0),       // Y축 벡터 (XY 평면에 눕혀서 그리기 위한 방향 설정)
//			FVector(1, 0, 0),       // X축 벡터
//			false                   // 축(Axis) 라인 그리기 여부
//		);
//
//		// [선택 사항] 몬스터 머리 위에 이름표와 가시거리 수치를 텍스트로 띄워주면 디버깅이 더 편합니다.
//		FString StatusText = FString::Printf(TEXT("%s\nDist: %.0f"), *Info.ActorName, Info.CullDistance);
//		DrawDebugString(GetWorld(), Info.Location + FVector(0, 0, 100.0f), StatusText, nullptr, DrawColor, 0.0f, false, 1.2f);
//	}
//
//	// =========================================================
//	// 3. (옵션) 맵 전체의 2D 공간 분할 격자선(Grid) 렌더링
//	// =========================================================
//	/*
//	// 엔진이 실제로 공간을 어떻게 쪼개고 있는지 눈으로 보고 싶다면,
//	// GridSubsystem->SharedBuffer.GridWidth 등을 활용하여 DrawDebugLine으로 바둑판을 그리는 로직을 이곳에 추가할 수 있습니다.
//	*/
//}