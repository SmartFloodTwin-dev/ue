#include "GeoTIFFReaderActor.h"
#include "Misc/Paths.h"

#include "gdal.h"
#include "gdal_priv.h"

#include <cmath>
#include <limits>

AGeoTIFFReaderActor::AGeoTIFFReaderActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 建立動態水面 Mesh 元件
    WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(
        TEXT("WaterMesh")
    );

    // 將 Mesh 設為 Actor 的 Root Component
    SetRootComponent(WaterMesh);

    // 暫時不啟用碰撞
    WaterMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GeoTIFF TEST] Constructor called")
    );
}


void AGeoTIFFReaderActor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[GeoTIFF TEST] BeginPlay called"));

    // 1. 載入 PINN 水深
    LoadGeoTIFF();

    // 2. 載入 DEM
    LoadTerrainGeoTIFF();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GeoTIFF TEST] Water Loaded: %s"),
        bRasterLoaded ? TEXT("YES") : TEXT("NO")
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GeoTIFF TEST] DEM Loaded: %s"),
        bTerrainLoaded ? TEXT("YES") : TEXT("NO")
    );

    if (!bRasterLoaded || !bTerrainLoaded)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[GeoTIFF TEST] Raster loading failed.")
        );

        return;
    }

    // 3. 測試中心像素
    const int32 TestCol = 370;
    const int32 TestRow = 239;

    // 4. 計算水深像素中心的地理座標
    const double WorldX =
        GeoTransform[0] +
        (TestCol + 0.5) * GeoTransform[1] +
        (TestRow + 0.5) * GeoTransform[2];

    const double WorldY =
        GeoTransform[3] +
        (TestCol + 0.5) * GeoTransform[4] +
        (TestRow + 0.5) * GeoTransform[5];

    // 5. 取得 DEM 高程
    float TerrainElevation = 0.0f;

    if (SampleTerrainBilinear(
        WorldX,
        WorldY,
        TerrainElevation))
    {
        const int32 WaterIndex =
    TestRow * RasterWidth + TestCol;

    if (!RasterData.IsValidIndex(WaterIndex))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Test] Invalid water raster index.")
        );

        return;
    }

    const float WaterDepth = RasterData[WaterIndex];

    if (!std::isfinite(WaterDepth))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] Water pixel is NoData.")
        );

        return;
    }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("========== Terrain Sampling Test ==========")
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] Pixel: Col=%d Row=%d"),
            TestCol,
            TestRow
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] World: X=%.3f Y=%.3f"),
            WorldX,
            WorldY
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] Terrain Elevation: %.3f m"),
            TerrainElevation
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] Water Depth: %.6f m"),
            WaterDepth
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Test] Water Surface: %.3f m"),
            TerrainElevation + WaterDepth
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("========== Sampling Test Completed ==========")
        );

        // 建立局部水面測試網格
        GenerateWaterMesh(
            350,  // 起始 Col
            219,  // 起始 Row
            40,   // 寬度：40 pixels
            40    // 高度：40 pixels
        );

        GenerateWaterMeshTest();
    }
    else
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Test] DEM bilinear sampling failed.")
        );
    }
}


void AGeoTIFFReaderActor::LoadGeoTIFF()
{
    bRasterLoaded = false;
    RasterData.Reset();
    ValidPixelCount = 0;
    UE_LOG(LogTemp, Warning,
        TEXT("========== GDAL GeoTIFF Reader =========="));

    // 1. 註冊 GDAL Drivers
    GDALAllRegister();

    // 2. 確認檔案存在
    UE_LOG(LogTemp, Warning,
        TEXT("[GDAL DEBUG] GeoTIFF Path: %s"), *GeoTIFFPath);

    const bool bFileExists = FPaths::FileExists(GeoTIFFPath);

    UE_LOG(LogTemp, Warning,
        TEXT("[GDAL DEBUG] File Exists: %s"),
        bFileExists ? TEXT("YES") : TEXT("NO"));

    if (!bFileExists)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GeoTIFF file not found: %s"), *GeoTIFFPath);
        return;
    }

    // 3. 開啟 GeoTIFF
    const char* FilePathUTF8 = TCHAR_TO_UTF8(*GeoTIFFPath);

    GDALDatasetH Dataset = GDALOpen(FilePathUTF8, GA_ReadOnly);

    UE_LOG(LogTemp, Warning,
        TEXT("[GDAL DEBUG] GDALOpen Result: %s"),
        Dataset != nullptr ? TEXT("SUCCESS") : TEXT("FAILED"));

    if (Dataset == nullptr)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GDALOpen failed: %s"), *GeoTIFFPath);
        return;
    }

    // 4. 取得 Raster 尺寸
    RasterWidth = GDALGetRasterXSize(Dataset);
    RasterHeight = GDALGetRasterYSize(Dataset);

    UE_LOG(LogTemp, Warning,
        TEXT("Raster Size: %d x %d"),
        RasterWidth, RasterHeight);

    // 5. 取得地理座標資訊
    

        if (GDALGetGeoTransform(Dataset, GeoTransform) != CE_None)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[GDAL] GetGeoTransform failed.")
        );

        GDALClose(Dataset);
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GDAL] Origin X: %.3f"),
        GeoTransform[0]
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GDAL] Origin Y: %.3f"),
        GeoTransform[3]
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GDAL] Pixel Width: %.3f"),
        GeoTransform[1]
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[GDAL] Pixel Height: %.3f"),
        GeoTransform[5]
    );

    // 6. 取得投影資訊
    const char* Projection = GDALGetProjectionRef(Dataset);

    if (Projection != nullptr)
    {
        FString ProjectionString = UTF8_TO_TCHAR(Projection);

        UE_LOG(LogTemp, Warning,
            TEXT("Projection: %s"),
            *ProjectionString.Left(500));
    }

    // 7. 取得第一個 Raster Band
    GDALRasterBandH Band = GDALGetRasterBand(Dataset, 1);

    if (Band == nullptr)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Cannot access raster band 1."));

        GDALClose(Dataset);
        return;
    }

    // 8. 取得 NoData 值
    int bHasNoData = FALSE;

    const double NoDataValue =
        GDALGetRasterNoDataValue(Band, &bHasNoData);

    if (bHasNoData)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("NoData Value: %.8f"), NoDataValue);
    }

    // 9. 檢查 Raster 尺寸
    const int64 PixelCount =
        static_cast<int64>(RasterWidth) * RasterHeight;

    if (RasterWidth <= 0 ||
        RasterHeight <= 0 ||
        PixelCount > MAX_int32)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Invalid raster dimensions."));

        GDALClose(Dataset);
        return;
    }

    // 10. 讀取 Raster 像素
    RasterData.SetNumUninitialized(static_cast<int32>(PixelCount));

    const CPLErr ReadResult = GDALRasterIO(
        Band,
        GF_Read,
        0, 0,
        RasterWidth,
        RasterHeight,
        RasterData.GetData(),
        RasterWidth,
        RasterHeight,
        GDT_Float32,
        0, 0
    );

    if (ReadResult != CE_None)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GDALRasterIO failed."));

        GDALClose(Dataset);
        return;
    }

    // 11. 計算有效像素統計
    double Sum = 0.0;

    float LocalMin = std::numeric_limits<float>::max();
    float LocalMax = std::numeric_limits<float>::lowest();

    ValidPixelCount = 0;

    for (const float Value : RasterData)
    {
        if (!std::isfinite(Value))
        {
            continue;
        }

        if (bHasNoData)
        {
            if (std::isnan(NoDataValue) && std::isnan(Value))
            {
                continue;
            }

            if (Value == NoDataValue)
            {
                continue;
            }
        }

        LocalMin = FMath::Min(LocalMin, Value);
        LocalMax = FMath::Max(LocalMax, Value);

        Sum += static_cast<double>(Value);
        ValidPixelCount++;
    }

    // 12. 輸出統計結果
    if (ValidPixelCount > 0)
    {
        bRasterLoaded = true;

        FVector TestPosition;

    if (RasterPixelToLocalUE(0, 0, TestPosition))
    {
        UE_LOG(LogTemp, Warning,
        TEXT("[Coordinate] Top-left pixel: %s"),
        *TestPosition.ToString());
    }

    if (RasterPixelToLocalUE(740, 479, TestPosition))
    {
        UE_LOG(LogTemp, Warning,
        TEXT("[Coordinate] Bottom-right pixel: %s"),
        *TestPosition.ToString());
    }

    if (RasterPixelToLocalUE(370, 239, TestPosition))
    {
        UE_LOG(LogTemp, Warning,
        TEXT("[Coordinate] Center pixel: %s"),
        *TestPosition.ToString());
    }

        UE_LOG(LogTemp, Warning,
        TEXT("[GDAL] Raster data saved in Actor."));

        UE_LOG(LogTemp, Warning,
        TEXT("[GDAL] Raster elements: %d"),
        RasterData.Num());
        
        MinDepth = LocalMin;
        MaxDepth = LocalMax;

        MeanDepth =
            static_cast<float>(Sum / ValidPixelCount);

        UE_LOG(LogTemp, Warning,
            TEXT("Valid Pixels: %d / %lld"),
            ValidPixelCount, PixelCount);

        UE_LOG(LogTemp, Warning,
            TEXT("Minimum Depth: %.6f m"), MinDepth);

        UE_LOG(LogTemp, Warning,
            TEXT("Maximum Depth: %.6f m"), MaxDepth);

        UE_LOG(LogTemp, Warning,
            TEXT("Mean Depth: %.6f m"), MeanDepth);

        UE_LOG(LogTemp, Warning,
            TEXT("Maximum Depth: %.2f cm"),
            MaxDepth * 100.0f);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("No valid raster pixels found."));
    }

    // 13. 關閉 Dataset
    GDALClose(Dataset);

    UE_LOG(LogTemp, Warning,
        TEXT("========== GDAL Read Completed =========="));
}


bool AGeoTIFFReaderActor::LoadTerrainGeoTIFF()
{
    UE_LOG(LogTemp, Warning, TEXT("========== Loading DEM =========="));

    bTerrainLoaded = false;
    TerrainData.Empty();

    GDALAllRegister();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Path: %s"),
        *TerrainGeoTIFFPath
    );

    if (!FPaths::FileExists(TerrainGeoTIFFPath))
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] File not found."));
        return false;
    }

    GDALDataset* Dataset = static_cast<GDALDataset*>(
        GDALOpen(
            TCHAR_TO_UTF8(*TerrainGeoTIFFPath),
            GA_ReadOnly
        )
    );

    if (Dataset == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] GDALOpen failed."));
        return false;
    }

    // 取得 Raster 尺寸
    TerrainWidth = Dataset->GetRasterXSize();
    TerrainHeight = Dataset->GetRasterYSize();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Size: %d x %d"),
        TerrainWidth,
        TerrainHeight
    );

    // 取得 GeoTransform
    if (Dataset->GetGeoTransform(TerrainGeoTransform) != CE_None)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] GetGeoTransform failed."));
        GDALClose(Dataset);
        return false;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Origin: X=%.3f Y=%.3f"),
        TerrainGeoTransform[0],
        TerrainGeoTransform[3]
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Pixel Size: X=%.3f Y=%.3f"),
        TerrainGeoTransform[1],
        TerrainGeoTransform[5]
    );

    // 取得 Band 1
    GDALRasterBand* Band = Dataset->GetRasterBand(1);

    if (Band == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] Raster band not found."));
        GDALClose(Dataset);
        return false;
    }

    // 取得 NoData
    int bHasNoData = FALSE;

    TerrainNoDataValue = Band->GetNoDataValue(&bHasNoData);
    bTerrainHasNoData = (bHasNoData != FALSE);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Has NoData: %s, Value: %.6f"),
        bTerrainHasNoData ? TEXT("YES") : TEXT("NO"),
        TerrainNoDataValue
    );

    // 配置記憶體
    const int64 TotalPixels =
        static_cast<int64>(TerrainWidth) * TerrainHeight;

    if (TotalPixels <= 0 ||
        TotalPixels > MAX_int32)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] Invalid raster size."));
        GDALClose(Dataset);
        return false;
    }

    TerrainData.SetNumUninitialized(
        static_cast<int32>(TotalPixels)
    );

    // 讀取完整 DEM
    const CPLErr ReadResult = Band->RasterIO(
        GF_Read,
        0,
        0,
        TerrainWidth,
        TerrainHeight,
        TerrainData.GetData(),
        TerrainWidth,
        TerrainHeight,
        GDT_Float32,
        0,
        0
    );

    GDALClose(Dataset);

    if (ReadResult != CE_None)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] RasterIO failed."));
        TerrainData.Empty();
        return false;
    }

    // 計算 DEM 統計資料
    double Sum = 0.0;
    float MinElevation = TNumericLimits<float>::Max();
    float MaxElevation = TNumericLimits<float>::Lowest();
    int32 ValidCount = 0;

    for (const float Value : TerrainData)
    {
        if (!std::isfinite(Value))
        {
            continue;
        }

        if (bTerrainHasNoData &&
            (
                (std::isnan(TerrainNoDataValue) && std::isnan(Value)) ||
                Value == TerrainNoDataValue
            ))
        {
            continue;
        }

        MinElevation = FMath::Min(MinElevation, Value);
        MaxElevation = FMath::Max(MaxElevation, Value);

        Sum += Value;
        ++ValidCount;
    }

    if (ValidCount == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[DEM] No valid elevation data."));
        TerrainData.Empty();
        return false;
    }

    bTerrainLoaded = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Valid Pixels: %d / %lld"),
        ValidCount,
        TotalPixels
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Min: %.3f m, Max: %.3f m, Mean: %.3f m"),
        MinElevation,
        MaxElevation,
        Sum / ValidCount
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[DEM] Data loaded successfully.")
    );

    UE_LOG(LogTemp, Warning, TEXT("========== DEM Loaded =========="));

    return true;
}


bool AGeoTIFFReaderActor::SampleTerrainBilinear(
    double WorldX,
    double WorldY,
    float& OutElevation
) const
{
    if (!bTerrainLoaded || TerrainData.IsEmpty())
    {
        return false;
    }

    // 將地理座標轉換成 DEM 像素中心索引
    const double PixelX =
        (WorldX - TerrainGeoTransform[0]) /
        TerrainGeoTransform[1] - 0.5;

    const double PixelY =
        (WorldY - TerrainGeoTransform[3]) /
        TerrainGeoTransform[5] - 0.5;

    // 找出周圍四個像素
    const int32 X0 = static_cast<int32>(std::floor(PixelX));
    const int32 Y0 = static_cast<int32>(std::floor(PixelY));

    const int32 X1 = X0 + 1;
    const int32 Y1 = Y0 + 1;

    // 邊界檢查
    if (X0 < 0 || Y0 < 0 ||
        X1 >= TerrainWidth ||
        Y1 >= TerrainHeight)
    {
        return false;
    }

    // 取得內插權重
    const double FX = PixelX - X0;
    const double FY = PixelY - Y0;

    auto GetElevation = [this](int32 X, int32 Y, float& Value) -> bool
    {
        const int32 Index = Y * TerrainWidth + X;

        if (!TerrainData.IsValidIndex(Index))
        {
            return false;
        }

        Value = TerrainData[Index];

        if (!std::isfinite(Value))
        {
            return false;
        }

        if (bTerrainHasNoData)
        {
            if (std::isnan(TerrainNoDataValue))
            {
                if (std::isnan(Value))
                {
                    return false;
                }
            }
            else if (Value == TerrainNoDataValue)
            {
                return false;
            }
        }

        return true;
    };

    float Z00, Z10, Z01, Z11;

    if (!GetElevation(X0, Y0, Z00) ||
        !GetElevation(X1, Y0, Z10) ||
        !GetElevation(X0, Y1, Z01) ||
        !GetElevation(X1, Y1, Z11))
    {
        return false;
    }

    // 雙線性內插
    const double Z =
        Z00 * (1.0 - FX) * (1.0 - FY) +
        Z10 * FX * (1.0 - FY) +
        Z01 * (1.0 - FX) * FY +
        Z11 * FX * FY;

    OutElevation = static_cast<float>(Z);

    return true;
}


void AGeoTIFFReaderActor::GenerateWaterMesh(
    int32 StartCol,
    int32 StartRow,
    int32 RegionWidth,
    int32 RegionHeight
)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("========== Generating Water Mesh ==========")
    );

    if (!bRasterLoaded || !bTerrainLoaded || !WaterMesh)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] Raster, DEM or WaterMesh is unavailable.")
        );
        return;
    }

    if (RegionWidth < 2 || RegionHeight < 2 ||
        StartCol < 0 || StartRow < 0 ||
        StartCol + RegionWidth > RasterWidth ||
        StartRow + RegionHeight > RasterHeight)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] Invalid test region.")
        );
        return;
    }

    TArray<FVector> Vertices;
    TArray<int32> Triangles;

    // 每個像素對應一個頂點索引。
    // INDEX_NONE 代表該像素無有效水深。
    TArray<int32> PixelToVertex;
    PixelToVertex.Init(
        INDEX_NONE,
        RegionWidth * RegionHeight
    );

    int32 ValidVertexCount = 0;

    // ------------------------------------------------
    // 1. 建立頂點
    // ------------------------------------------------

    for (int32 Row = 0; Row < RegionHeight; Row++)
    {
        for (int32 Col = 0; Col < RegionWidth; Col++)
        {
            const int32 RasterCol = StartCol + Col;
            const int32 RasterRow = StartRow + Row;

            const int32 WaterIndex =
                RasterRow * RasterWidth + RasterCol;

            if (!RasterData.IsValidIndex(WaterIndex))
            {
                continue;
            }

            const float WaterDepth = RasterData[WaterIndex];

            // 排除 NoData
            if (!std::isfinite(WaterDepth))
            {
                continue;
            }

            // 計算水深像素中心的 TWD97 座標
            const double WorldX =
                GeoTransform[0] +
                (RasterCol + 0.5) * GeoTransform[1] +
                (RasterRow + 0.5) * GeoTransform[2];

            const double WorldY =
                GeoTransform[3] +
                (RasterCol + 0.5) * GeoTransform[4] +
                (RasterRow + 0.5) * GeoTransform[5];

            // 對應 DEM 地形高程
            float TerrainElevation = 0.0f;

            if (!SampleTerrainBilinear(
                WorldX,
                WorldY,
                TerrainElevation
            ))
            {
                continue;
            }

            // 轉換成 UE Local Coordinates
            FVector LocalPosition;

            if (!RasterPixelToLocalUE(
                RasterCol,
                RasterRow,
                LocalPosition
            ))
            {
                continue;
            }

            // 地形高程 + 水深，單位 m 轉成 UE cm
            const double WaterSurfaceElevation =
                TerrainElevation + WaterDepth;

            LocalPosition.Z =
                WaterSurfaceElevation * 100.0;

            const int32 VertexIndex = Vertices.Add(
                LocalPosition
            );

            const int32 LocalPixelIndex =
                Row * RegionWidth + Col;

            PixelToVertex[LocalPixelIndex] =
                VertexIndex;

            ValidVertexCount++;
        }
    }

    // ------------------------------------------------
    // 2. 建立三角形
    // ------------------------------------------------

    int32 TriangleCount = 0;

    for (int32 Row = 0; Row < RegionHeight - 1; Row++)
    {
        for (int32 Col = 0; Col < RegionWidth - 1; Col++)
        {
            const int32 TL =
                PixelToVertex[Row * RegionWidth + Col];

            const int32 TR =
                PixelToVertex[Row * RegionWidth + Col + 1];

            const int32 BL =
                PixelToVertex[(Row + 1) * RegionWidth + Col];

            const int32 BR =
                PixelToVertex[(Row + 1) * RegionWidth + Col + 1];

            // 四個角落都必須有有效頂點
            if (TL == INDEX_NONE ||
                TR == INDEX_NONE ||
                BL == INDEX_NONE ||
                BR == INDEX_NONE)
            {
                continue;
            }

            // 第一個三角形
            Triangles.Add(TL);
            Triangles.Add(BL);
            Triangles.Add(TR);

            // 第二個三角形
            Triangles.Add(TR);
            Triangles.Add(BL);
            Triangles.Add(BR);

            TriangleCount += 2;
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Valid Vertices: %d"),
        ValidVertexCount
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Triangles: %d"),
        TriangleCount
    );

    if (Vertices.Num() == 0 || Triangles.Num() == 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] No valid mesh geometry generated.")
        );
        return;
    }

    // ------------------------------------------------
    // 3. 建立 Procedural Mesh Section
    // ------------------------------------------------

    WaterMesh->ClearAllMeshSections();

    WaterMesh->CreateMeshSection(
        0,
        Vertices,
        Triangles,
        TArray<FVector>(),
        TArray<FVector2D>(),
        TArray<FColor>(),
        TArray<FProcMeshTangent>(),
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Water mesh generated successfully.")
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("========== Water Mesh Completed ==========")
    );
}



void AGeoTIFFReaderActor::GenerateWaterMeshTest()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("========== Generate Water Mesh Test ==========")
    );

    if (!WaterMesh || !bRasterLoaded || !bTerrainLoaded)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] Required components or raster data missing.")
        );
        return;
    }

    // 測試區域：40 x 40 pixels
    const int32 CenterCol = 370;
    const int32 CenterRow = 239;

    const int32 PatchWidth = 40;
    const int32 PatchHeight = 40;

    const int32 StartCol = CenterCol - PatchWidth / 2;
    const int32 StartRow = CenterRow - PatchHeight / 2;

    const int32 EndCol = StartCol + PatchWidth;
    const int32 EndRow = StartRow + PatchHeight;

    if (StartCol < 0 || StartRow < 0 ||
        EndCol > RasterWidth || EndRow > RasterHeight)
    {
        UE_LOG(LogTemp, Error, TEXT("[Mesh] Patch out of bounds."));
        return;
    }

    const int32 VertexCount = PatchWidth * PatchHeight;

    TArray<FVector> Vertices;
    TArray<FVector2D> UV0;
    TArray<FColor> VertexColors;
    TArray<uint8> ValidMask;
    TArray<int32> Triangles;

    Vertices.SetNumZeroed(VertexCount);
    UV0.SetNumZeroed(VertexCount);
    VertexColors.SetNumZeroed(VertexCount);
    ValidMask.Init(0, VertexCount);

    // 以測試區域中心作為 Local XY 原點
    FVector CenterPosition;

    if (!RasterPixelToLocalUE(
        CenterCol,
        CenterRow,
        CenterPosition))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] Failed to calculate center position.")
        );
        return;
    }

    int32 ValidVertexCount = 0;

    // 第一階段：建立頂點
    for (int32 Row = 0; Row < PatchHeight; Row++)
    {
        for (int32 Col = 0; Col < PatchWidth; Col++)
        {
            const int32 RasterCol = StartCol + Col;
            const int32 RasterRow = StartRow + Row;

            const int32 RasterIndex =
                RasterRow * RasterWidth + RasterCol;

            const int32 VertexIndex =
                Row * PatchWidth + Col;

            if (!RasterData.IsValidIndex(RasterIndex))
            {
                continue;
            }

            const float WaterDepth = RasterData[RasterIndex];

            // 排除 NaN / Inf / NoData
            if (!std::isfinite(WaterDepth))
            {
                continue;
            }

            // 取得水深像素中心的 TWD97 座標
            const double WorldX =
                GeoTransform[0] +
                (RasterCol + 0.5) * GeoTransform[1] +
                (RasterRow + 0.5) * GeoTransform[2];

            const double WorldY =
                GeoTransform[3] +
                (RasterCol + 0.5) * GeoTransform[4] +
                (RasterRow + 0.5) * GeoTransform[5];

            // DEM 雙線性插值
            float TerrainElevation = 0.0f;

            if (!SampleTerrainBilinear(
                WorldX,
                WorldY,
                TerrainElevation))
            {
                continue;
            }

            // 水面高程（公尺）
            const float WaterSurfaceElevation =
                TerrainElevation + WaterDepth;

            // 轉換成 UE Local Coordinates
            FVector LocalPosition;

            if (!RasterPixelToLocalUE(
                RasterCol,
                RasterRow,
                LocalPosition))
            {
                continue;
            }

            // 測試用：將測試區域中心移至 Actor 原點
            LocalPosition.X -= CenterPosition.X;
            LocalPosition.Y -= CenterPosition.Y;

            // 公尺轉換成 UE 公分
            LocalPosition.Z = WaterSurfaceElevation * 100.0f;

            Vertices[VertexIndex] = LocalPosition;

            UV0[VertexIndex] = FVector2D(
                static_cast<float>(Col) / (PatchWidth - 1),
                static_cast<float>(Row) / (PatchHeight - 1)
            );

            VertexColors[VertexIndex] =
                FColor(0, 120, 255, 255);

            ValidMask[VertexIndex] = 1;

            ValidVertexCount++;
        }
    }

    // 第二階段：建立三角形
    int32 ValidCellCount = 0;

    for (int32 Row = 0; Row < PatchHeight - 1; Row++)
    {
        for (int32 Col = 0; Col < PatchWidth - 1; Col++)
        {
            const int32 TopLeft =
                Row * PatchWidth + Col;

            const int32 TopRight =
                TopLeft + 1;

            const int32 BottomLeft =
                (Row + 1) * PatchWidth + Col;

            const int32 BottomRight =
                BottomLeft + 1;

            // 四個頂點都有效才建立面
            if (!ValidMask[TopLeft] ||
                !ValidMask[TopRight] ||
                !ValidMask[BottomLeft] ||
                !ValidMask[BottomRight])
            {
                continue;
            }

            // 第一個三角形
            Triangles.Add(TopLeft);
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);

            // 第二個三角形
            Triangles.Add(TopRight);
            Triangles.Add(BottomLeft);
            Triangles.Add(BottomRight);

            ValidCellCount++;
        }
    }

    if (Triangles.Num() == 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Mesh] No valid triangles generated.")
        );
        return;
    }

    // 第三階段：建立 Procedural Mesh
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;

    WaterMesh->ClearAllMeshSections();

    WaterMesh->CreateMeshSection(
        0,
        Vertices,
        Triangles,
        Normals,
        UV0,
        VertexColors,
        Tangents,
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Valid Vertices: %d / %d"),
        ValidVertexCount,
        VertexCount
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Valid Cells: %d"),
        ValidCellCount
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Triangle Count: %d"),
        Triangles.Num() / 3
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Mesh] Water Mesh Created Successfully.")
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("========== Water Mesh Test Completed ==========")
    );
}


bool AGeoTIFFReaderActor::RasterPixelToLocalUE(
    int32 Col,
    int32 Row,
    FVector& OutPosition
) const
{
    // 確認 Raster 已經成功載入
    if (!bRasterLoaded)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[Coordinate] Raster not loaded."));
        return false;
    }

    // 確認像素索引合法
    if (Col < 0 || Col >= RasterWidth ||
        Row < 0 || Row >= RasterHeight)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[Coordinate] Invalid pixel: Col=%d Row=%d"),
            Col, Row);
        return false;
    }

    // 使用像素中心位置，避免使用左上角邊界
    const double PixelCenterX =
        GeoTransform[0] +
        (Col + 0.5) * GeoTransform[1] +
        (Row + 0.5) * GeoTransform[2];

    const double PixelCenterY =
        GeoTransform[3] +
        (Col + 0.5) * GeoTransform[4] +
        (Row + 0.5) * GeoTransform[5];

    // 轉換成相對於 GeoTIFF 左上角的 UE 座標
    // 地理座標單位：公尺
    // UE 座標單位：公分
    const double LocalX =
        (PixelCenterX - GeoTransform[0]) * 100.0;

    const double LocalY =
        (PixelCenterY - GeoTransform[3]) * 100.0;

    OutPosition = FVector(
        LocalX,
        LocalY,
        0.0
    );

    return true;
}


void AGeoTIFFReaderActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
