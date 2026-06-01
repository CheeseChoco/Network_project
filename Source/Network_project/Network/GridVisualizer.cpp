#include "GridVisualizer.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NPGridSubsystem.h" // 서브시스템 헤더 포함

AGridVisualizer::AGridVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
}

void AGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	// 1. 시각화 액터가 태어날 때 서브시스템을 초기화해줍니다.
	// (실제 프로젝트에서는 맵의 전체 넓이(예: 30000x30000)를 계산해서 넣어주면 좋습니다.)
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UNPGridSubsystem* GridSubsystem = GameInstance->GetSubsystem<UNPGridSubsystem>())
		{
			// 예시: 가로 30000, 세로 30000 크기의 월드를 기준으로 버퍼 메모리 사전 할당
			GridSubsystem->InitGridSystem(30000.0f, 30000.0f, GridCellSize);
		}
	}
}

bool AGridVisualizer::ShouldTickIfViewportsOnly() const
{
	return true;
}

FIntPoint AGridVisualizer::WorldToGridIndex(const FVector& WorldLocation) const
{
	int32 IndexX = FMath::FloorToInt(WorldLocation.X / GridCellSize);
	int32 IndexY = FMath::FloorToInt(WorldLocation.Y / GridCellSize);
	return FIntPoint(IndexX, IndexY);
}

FVector AGridVisualizer::GridIndexToWorldCenter(const FIntPoint& GridIndex) const
{
	float CenterX = (GridIndex.X * GridCellSize) + (GridCellSize * 0.5f);
	float CenterY = (GridIndex.Y * GridCellSize) + (GridCellSize * 0.5f);
	return FVector(CenterX, CenterY, DrawZHeight);
}

void AGridVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UNPGridSubsystem* GridSubsystem = GameInstance->GetSubsystem<UNPGridSubsystem>();
	if (!GridSubsystem || GridSubsystem->SharedBuffer.TotalCells == 0) return;

	// 1. 락(Lock) 없이 안전하게 앞배경 읽어오기
	const TArray<uint8>& FrontBuffer = GridSubsystem->SharedBuffer.GetFrontBuffer();

	int32 Width = GridSubsystem->SharedBuffer.GridWidth;
	int32 Height = GridSubsystem->SharedBuffer.GridHeight;
	float MarginScale = 0.99f;
	FVector BoxExtent((GridCellSize / 2.0f) * MarginScale, (GridCellSize / 2.0f) * MarginScale, 10.0f);

	// 2. 전체 그리드를 순회하며 활성화된 구역만 그리기
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 Index = GridSubsystem->SharedBuffer.GetIndex(X, Y);
			if (Index == INDEX_NONE) continue;

			uint8 Intensity = FrontBuffer[Index];

			// 강도가 0보다 클 때만 (즉, 이 칸에 액터가 존재하거나 갱신되었을 때만) 박스 그리기
			if (Intensity > 0)
			{
				// 1. 그라데이션을 위한 Alpha 값 계산 (0.0 ~ 1.0)
				float Alpha = FMath::Clamp(Intensity / 255.0f, 0.0f, 1.0f);

				// 2. [수정됨] 실수형인 FLinearColor를 사용하여 부드럽게 색상 보간(Lerp)
				FLinearColor LerpedColor = FMath::Lerp(FLinearColor::Yellow, FLinearColor::Red, Alpha);

				// 3. DrawDebugBox에 넣기 위해 다시 FColor로 변환 (true는 sRGB 변환 적용 여부)
				FColor DrawColor = LerpedColor.ToFColor(true);

				FVector BoxCenter = GridIndexToWorldCenter(FIntPoint(X, Y));

				// 디버그 박스 그리기
				DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, FQuat::Identity, DrawColor, false, 0.1f, 0, 10.0f);
			}
		}
	}
}