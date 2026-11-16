#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TimerManager.h"
#include "GeoTIFFReaderActor.generated.h"

UCLASS()
class MYPROJECT_API AGeoTIFFReaderActor : public AActor
{
    GENERATED_BODY()

public:
    AGeoTIFFReaderActor();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flood Mesh")
    TObjectPtr<UProceduralMeshComponent> WaterMesh;

    // 儲存完整淹水區域的所有 Mesh 區塊
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flood Mesh")
    TArray<TObjectPtr<UProceduralMeshComponent>> WaterMeshChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Mesh")
    UMaterialInterface* WaterMaterial;

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Flood Animation
    void LoadAndGenerateFrame(int32 FrameIndex);
    void ClearWaterMesh();
    void GenerateFullWaterMesh();
    void AdvanceToNextFrame();

    FTimerHandle AnimationTimerHandle;
    int32 CurrentFrameIndex = 0;

    // Water Mesh
    void GenerateWaterMesh(
        int32 StartCol,
        int32 StartRow,
        int32 RegionWidth,
        int32 RegionHeight
    );

    bool RasterPixelToLocalUE(
        int32 Col,
        int32 Row,
        FVector& OutPosition
    ) const;

    void GenerateWaterMeshTest();

    // GeoTIFF
    void LoadGeoTIFF();
    bool LoadTerrainGeoTIFF();

    bool SampleTerrainBilinear(
        double WorldX,
        double WorldY,
        float& OutElevation
    ) const;

    TArray<float> RasterData;
    double GeoTransform[6] = {};

    TArray<float> TerrainData;
    double TerrainGeoTransform[6] = {};

    int32 TerrainWidth = 0;
    int32 TerrainHeight = 0;

    double TerrainNoDataValue = 0.0;
    bool bTerrainHasNoData = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GeoTIFF")
    bool bTerrainLoaded = false;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GDAL")
    FString GeoTIFFPath =
        TEXT("C:/Users/jeff2/Desktop/PINN/outputs/minxiong/prediction/depth_012.tif");

    // DEM GeoTIFF 路徑
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeoTIFF")
    FString TerrainGeoTIFFPath =
        TEXT("C:/Users/jeff2/processing/Minxiong_DEM_20m.tif");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    int32 RasterWidth = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    int32 RasterHeight = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    int32 ValidPixelCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    float MinDepth = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    float MaxDepth = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    float MeanDepth = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GDAL")
    bool bRasterLoaded = false;

        // ================================
        // Flood Animation Settings
        // ================================

        // PINN 預測結果資料夾
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Animation")
        FString PredictionDirectory =
            TEXT("C:/Users/jeff2/Desktop/PINN/outputs/minxiong/prediction");

        // 總時間點數量
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Animation")
        int32 TotalFrameCount = 21;

        // 每個時間點停留秒數
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Animation")
        float FrameIntervalSeconds = 1.0f;

        // 是否在遊戲開始時自動播放
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Animation")
        bool bAutoPlay = true;
};
