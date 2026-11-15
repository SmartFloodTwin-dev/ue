#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "GeoTIFFReaderActor.generated.h"

UCLASS()
class MYPROJECT_API AGeoTIFFReaderActor : public AActor
{
    GENERATED_BODY()

public:
    AGeoTIFFReaderActor();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flood Mesh")
    TObjectPtr<UProceduralMeshComponent> WaterMesh;

protected:
    void GenerateWaterMesh(
        int32 StartCol,
        int32 StartRow,
        int32 RegionWidth,
        int32 RegionHeight
    );

    // 將 Raster 像素座標轉換成 UE 區域座標
    bool RasterPixelToLocalUE(
        int32 Col,
        int32 Row,
        FVector& OutPosition
    ) const;

    void GenerateWaterMeshTest();
    
    virtual void BeginPlay() override;

    // 讀取 GeoTIFF
    void LoadGeoTIFF();

    // 載入 DEM GeoTIFF
    bool LoadTerrainGeoTIFF();

    // 根據實際地理座標取得 DEM 高程（雙線性內插）
    bool SampleTerrainBilinear(
        double WorldX,
        double WorldY,
        float& OutElevation
    ) const;

    // 保存完整 Raster 水深資料
    TArray<float> RasterData;

    // GeoTIFF 地理座標轉換參數
    // [0] 左上角 X
    // [1] 像素寬度
    // [2] X 方向旋轉
    // [3] 左上角 Y
    // [4] Y 方向旋轉
    // [5] 像素高度
    double GeoTransform[6] = {};

    // DEM 高程資料
    TArray<float> TerrainData;

    // DEM 地理座標轉換資訊
    double TerrainGeoTransform[6] = {};

    // DEM Raster 尺寸
    int32 TerrainWidth = 0;
    int32 TerrainHeight = 0;

    // DEM NoData
    double TerrainNoDataValue = 0.0;
    bool bTerrainHasNoData = false;

    // DEM 載入狀態
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
};
