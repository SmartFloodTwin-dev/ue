# Unreal Engine 專案操作 README

## 1. 專案簡介

本專案使用 Unreal Engine 建置嘉義縣民雄鄉智慧城市數位孿生淹水模擬平台，負責將地理資訊系統（GIS）與 PINN（Physics-Informed Neural Network）產生的淹水預測結果整合至 Unreal Engine，建立可視化的三維地形與動態水面。

目前 UE 模組主要負責：

1. 載入民雄鄉地形資料（DEM）。
2. 讀取 PINN 輸出的淹水深度 GeoTIFF。
3. 將 GeoTIFF 像素座標轉換為 Unreal Engine 世界座標。
4. 根據 DEM 高程與淹水深度計算水面高度。
5. 使用 C++ Procedural Mesh 動態建立水面。
6. 將大型水面切分為多個 Mesh Chunk。
7. 依序載入 21 個不同時間點的淹水預測結果。
8. 使用 Unreal Engine Timer 控制水面動畫播放。

### 1.1 開發環境

| 項目 | 版本 / 設定 |
|---|---|
| Unreal Engine | 5.8.3 |
| IDE | Visual Studio 2022 Community |
| 專案名稱 | MyProject |
| 程式語言 | C++ |
| 地理資料處理函式庫 | GDAL |
| 地理座標系統 | TWD97 / TM2 zone 121 |
| EPSG | EPSG:3826 |
| 地形資料 | 民雄 20 m DEM |
| 淹水預測資料 | PINN GeoTIFF |
| 水面生成方式 | Procedural Mesh Component |
| 動畫控制方式 | Unreal Engine Timer |

---

## 2. 開啟 Unreal Engine 專案

### 2.1 專案位置

目前專案路徑：

```text
C:\Users\jeff2\Documents\Unreal Projects\MyProject\
```

主要專案檔案：

```text
MyProject.uproject
```

### 2.2 開啟步驟

1. 開啟 Windows 檔案總管。
2. 前往 Unreal Engine 專案資料夾。
3. 找到 `MyProject.uproject`。
4. 雙擊專案檔案。
5. 等待 Unreal Engine 5.8.3 啟動。
6. 等待 Shader Compilation、資產載入及必要的初始化工作完成。
7. 確認 Unreal Editor 正常進入編輯畫面。

### 2.3 開啟專案時的注意事項

- 第一次開啟專案可能需要較長的 Shader Compilation 時間。
- 若出現 C++ 編譯提示，應確認 Visual Studio 2022 與 Unreal Engine 的開發環境正常。
- 如果 GDAL 相關程式碼無法編譯，應先確認 ThirdParty/GDAL 的 Include、Library 與 DLL 路徑。
- 修改 C++ Header 或 CPP 後，應重新編譯專案。

---

## 3. 建立新的 Level（New Map）

### 3.1 建立地圖

1. 開啟 Unreal Editor。
2. 點選上方選單：

   `File → New Level`

3. 選擇適合的 Level 類型。
4. 若要從空白場景開始建置，建議選擇 `Empty Level`。
5. 建立後確認目前 Level 已經切換至新的場景。

### 3.2 儲存 Level

1. 點選：

   `File → Save Current Level As`

2. 選擇專案的 `Content` 資料夾。
3. 輸入地圖名稱：

```text
Minxiong_Map
```

4. 儲存。

建議專案結構：

```text
MyProject/
├── Config/
├── Content/
│   ├── Maps/
│   │   └── Minxiong_Map.umap
│   ├── Materials/
│   ├── Meshes/
│   ├── Textures/
│   └── ...
├── Source/
│   └── MyProject/
├── ThirdParty/
│   └── GDAL/
└── MyProject.uproject
```

上述為建議整理方式，實際資料夾應依目前專案資產配置調整。

### 3.3 儲存注意事項

- 建立新 Level 後應立即儲存。
- 每次新增 Actor、調整場景或修改資產後，應適時儲存。
- 若更換 Level，應確認目前操作的地圖是否正確。
- 不要將尚未儲存的場景當成正式版本。

---

## 4. Unreal Editor 主要操作介面

Unreal Editor 主要分成以下幾個區域：

| 介面 | 功能 |
|---|---|
| Viewport | 顯示與編輯三維場景 |
| World Outliner | 管理目前 Level 中的 Actor |
| Details | 查看與修改選取 Actor 的屬性 |
| Content Drawer | 管理專案中的資產 |
| Toolbar | 提供 Play、Build、Save 等操作 |
| Output Log | 查看程式執行訊息、錯誤與除錯資訊 |
| Components | 查看 Actor 所包含的元件 |

### 4.1 World Outliner

World Outliner 位於 Unreal Editor 右側，用來管理目前 Level 中的所有 Actor。

目前場景建議依照以下架構整理：

```text
Minxiong_Map
│
├── GeoTIFFReaderActor
│   └── WaterMesh
│
├── DirectionalLight
├── SkyLight
├── SkyAtmosphere
├── SkySphere / SkyDome（若有使用）
├── ExponentialHeightFog（若有使用）
└── 其他場景物件
```

注意：

1. `GeoTIFFReaderActor` 是負責讀取 GeoTIFF 與產生水面的主要 Actor。
2. `WaterMesh` 是掛在 `GeoTIFFReaderActor` 上的 Procedural Mesh Component。
3. 其他水面區塊會透過 C++ 動態建立為額外的 Procedural Mesh Component。
4. 動態建立的 Chunk 是 Actor 的 Components，不是獨立的 Level Actor，因此不一定會以獨立項目出現在 World Outliner。
5. DirectionalLight、SkyLight 等是場景照明與環境元件，並非淹水計算必要元件。

若場景中尚未建立地形、照明或其他物件，Outliner 不一定會包含以上所有項目。

### 4.2 GeoTIFFReaderActor 的建立方式

若場景中尚未放入 GeoTIFFReaderActor：

1. 開啟 Content Drawer。
2. 找到專案的 C++ Class。
3. 搜尋 `GeoTIFFReaderActor`。
4. 將 Actor 拖曳至 Viewport。
5. 確認 World Outliner 出現 `GeoTIFFReaderActor`。
6. 點選該 Actor，檢查右側 Details 面板。

若沒有看到該 C++ Class，應先確認專案是否成功編譯。

### 4.3 Components 面板

選取 `GeoTIFFReaderActor` 後，應可在 Components 區域看到：

| Component | 說明 |
|---|---|
| DefaultSceneRoot 或實際 Root Component | Actor 的根元件，依 Constructor 設定而定 |
| WaterMesh | 第一個水面 Mesh Chunk |

其他 Chunk 會在執行時依據 Raster 尺寸動態建立。

---

## 5. Content Drawer 與資產管理

Content Drawer 用來管理 Unreal Engine 專案中的資產。

開啟方式：

1. 點選 Unreal Editor 左下角的 `Content Drawer`。
2. 或使用快捷鍵 `Ctrl + Space`。
3. 在 Content Drawer 中瀏覽專案的 Content 資料夾。

### 5.1 民雄邊界資產

在 Content Drawer 中找到：

```text
民雄邊界
```

確認該資產可以正常開啟或顯示。

民雄邊界主要用途：

- 作為民雄鄉地理範圍參考。
- 確認地形資料涵蓋範圍。
- 協助判斷淹水預測結果的空間位置。
- 作為後續道路、建築物及其他 GIS 圖層整合的參考。

注意：民雄邊界資產的實際匯入格式及顯示方式，依 Content Drawer 中的資產類型而定。

### 5.2 建議資產分類

```text
Content/
│
├── Maps/
│   └── Minxiong_Map
│
├── Materials/
│   └── WaterMaterial
│
├── Meshes/
│
├── Textures/
│
├── Blueprints/
│
└── ...
```

以上是建議分類，實際資產名稱以專案目前內容為準。

---

## 6. 民雄鄉地理資料

### 6.1 地理資料設定

| 項目 | 設定 |
|---|---|
| 行政區域 | 嘉義縣民雄鄉 |
| 座標系統 | TWD97 / TM2 zone 121 |
| EPSG | EPSG:3826 |
| DEM 解析度 | 20 m |
| DEM 格式 | GeoTIFF |
| 淹水資料格式 | GeoTIFF |
| 水深資料型態 | Float32 |
| DEM 資料型態 | Float32 |

### 6.2 DEM 地形資料

目前使用的 DEM：

```text
C:\Users\jeff2\processing\Minxiong_DEM_20m.tif
```

Raster 資訊：

| 項目 | 數值 |
|---|---|
| Raster Width | 901 |
| Raster Height | 701 |
| Pixel Size X | 20 m |
| Pixel Size Y | -20 m |
| Data Type | Float32 |
| 座標系統 | EPSG:3826 |
| 最低高程 | 0 m |
| 最高高程 | 約 200.740 m |

DEM 用來提供每個水面像素對應的地形高程。

水面高度計算方式：

```text
Water Surface Elevation
    = Terrain Elevation + Water Depth
```

UE 使用公分作為一般世界座標的距離單位，因此轉換為：

```cpp
const float WaterSurfaceZ =
    (TerrainElevation + WaterDepth) * 100.0f;
```

其中：

- `TerrainElevation`：DEM 取得的地形高程，單位為公尺。
- `WaterDepth`：PINN 預測的水深，單位為公尺。
- `WaterSurfaceZ`：Unreal Engine 中的水面 Z 座標，單位為公分。

### 6.3 水深資料與 DEM 的空間對應

水深 GeoTIFF：

```text
741 × 480
```

DEM：

```text
901 × 701
```

兩者的 Raster 尺寸不同，因此不能直接使用相同的 Row、Col 索引讀取高程。

目前透過 GeoTransform 將水深 Raster 像素中心轉換為實際地理座標，再對 DEM 進行雙線性內插。

水深像素中心的地理座標：

```cpp
WorldX =
    GeoTransform[0] +
    (Col + 0.5) * GeoTransform[1] +
    (Row + 0.5) * GeoTransform[2];

WorldY =
    GeoTransform[3] +
    (Col + 0.5) * GeoTransform[4] +
    (Row + 0.5) * GeoTransform[5];
```

再使用 DEM 的 GeoTransform 計算對應位置。

目前已實作：

```cpp
SampleTerrainBilinear(
    WorldX,
    WorldY,
    OutElevation
);
```

這樣可以取得水深像素位置所對應的地形高程。

---

## 7. GDAL 第三方函式庫設定

本專案使用 GDAL 讀取 PINN 輸出的 GeoTIFF。

### 7.1 GDAL 安裝來源

目前使用 OSGeo4W 安裝的 GDAL。

原始安裝路徑：

```text
C:\OSGeo4W\
```

主要檔案：

| 項目 | 路徑 |
|---|---|
| Include | C:\OSGeo4W\include |
| Library | C:\OSGeo4W\lib\gdal.lib |
| DLL | C:\OSGeo4W\bin\gdal313.dll |
| GDAL Data | C:\OSGeo4W\apps\gdal\share\gdal |
| PROJ Data | C:\OSGeo4W\share\proj |

### 7.2 專案內 ThirdParty/GDAL

目前專案已整理 GDAL 相關檔案：

```text
MyProject/
└── ThirdParty/
    └── GDAL/
        ├── include/
        ├── lib/
        │   └── gdal.lib
        ├── bin/
        │   ├── gdal313.dll
        │   └── 其他相依 DLL
        └── data/
            ├── gdal/
            └── proj/
                └── proj.db
```

用途：

- `include`：提供 C++ 編譯時使用的 GDAL 標頭檔。
- `lib`：提供連結器使用的 GDAL Library。
- `bin`：提供程式執行時所需的 DLL。
- `data/gdal`：提供 GDAL 相關資料。
- `data/proj`：提供 PROJ 座標轉換資料。

### 7.3 GDAL 在程式中的使用

主要標頭檔：

```cpp
#include "gdal.h"
#include "gdal_priv.h"
```

註冊 GDAL：

```cpp
GDALAllRegister();
```

開啟 GeoTIFF：

```cpp
GDALDatasetH Dataset =
    GDALOpen(FilePathUTF8, GA_ReadOnly);
```

取得 Raster Band：

```cpp
GDALRasterBandH Band =
    GDALGetRasterBand(Dataset, 1);
```

讀取 Raster：

```cpp
GDALRasterIO(
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
```

關閉 Dataset：

```cpp
GDALClose(Dataset);
```

注意：若要在其他電腦執行專案，必須確認 GDAL DLL、相依 DLL 與資料目錄都能正常載入，不能只複製 `gdal.lib`。

---

## 8. Water 模組架構

目前 Water 模組以 C++ 自行實作，不依賴 Unreal Engine 內建 Water System 作為主要淹水資料來源。

### 8.1 主要 C++ 類別

主要類別：

```text
AGeoTIFFReaderActor
```

主要檔案：

```text
Source/
└── MyProject/
    ├── GeoTIFFReaderActor.h
    └── GeoTIFFReaderActor.cpp
```

主要功能：

| 函式 | 功能 |
|---|---|
| `BeginPlay()` | 初始化 DEM 並開始動畫播放 |
| `LoadGeoTIFF()` | 讀取指定的水深 GeoTIFF |
| `LoadTerrainGeoTIFF()` | 載入 DEM |
| `SampleTerrainBilinear()` | 使用雙線性內插取得地形高程 |
| `RasterPixelToLocalUE()` | 將 Raster 像素座標轉換為 UE 區域座標 |
| `GenerateWaterMesh()` | 建立指定 Raster 區域的水面 Mesh |
| `GenerateFullWaterMesh()` | 將整張 Raster 分塊並建立完整水面 |
| `ClearWaterMesh()` | 清除上一幀的水面 |
| `LoadAndGenerateFrame()` | 載入指定影格並重新生成水面 |
| `AdvanceToNextFrame()` | 切換至下一個動畫影格 |

### 8.2 WaterMesh

`WaterMesh` 是主要的 Procedural Mesh Component。

建立方式：

```cpp
CreateMeshSection(
    0,
    Vertices,
    Triangles,
    Normals,
    UV0,
    VertexColors,
    Tangents,
    false
);
```

主要用途：

- 儲存水面頂點。
- 儲存水面三角形索引。
- 建立動態水面幾何形狀。
- 指定水面材質。
- 更新不同時間點的水面。

### 8.3 WaterMeshChunks

由於水深 Raster 尺寸為 741 × 480，若一次建立完整 Mesh，會產生大量頂點與三角形。

因此目前採用分塊方式：

```cpp
const int32 ChunkSize = 100;
```

每個 Chunk 最多涵蓋：

```text
100 × 100 pixels
```

以目前 Raster 尺寸計算：

- 水平方向：8 個 Chunk。
- 垂直方向：5 個 Chunk。
- 總計：40 個 Chunk 區域。

最後一列與最後一行的 Chunk 會依剩餘 Raster 尺寸縮小。

Chunk 由 `GenerateWaterMesh()` 建立，並儲存在：

```cpp
TArray<TObjectPtr<UProceduralMeshComponent>>
    WaterMeshChunks;
```

這樣可以分區建立與清除水面。

### 8.4 水面像素篩選

目前水面生成時會排除：

```cpp
if (!std::isfinite(WaterDepth) || WaterDepth <= 0.001f)
{
    continue;
}
```

篩選條件：

1. 排除 NaN。
2. 排除 Infinity。
3. 排除小於或等於 0.001 m 的水深。
4. 只保留具有實際淹水深度的像素。

因此第 0 幀即使 Raster 有效像素存在，但水深全部為 0，仍不會建立水面。

### 8.5 水面三角形

每個有效像素會先建立對應頂點，再檢查相鄰四個像素是否都有效。

只有四個角點均存在時，才建立兩個三角形：

```cpp
Triangles.Add(TL);
Triangles.Add(BL);
Triangles.Add(TR);

Triangles.Add(TR);
Triangles.Add(BL);
Triangles.Add(BR);
```

這樣可以避免將無效像素或 NoData 區域連接成水面。

---

## 9. GeoTIFFReaderActor 的 Details 設定

選取 World Outliner 中的：

```text
GeoTIFFReaderActor
```

右側 Details 面板會顯示此 Actor 的可調整屬性。

目前主要分成以下分類：

1. Flood Mesh
2. GDAL
3. GeoTIFF
4. Flood Animation

實際顯示內容可能因 Unreal Engine 版本、C++ UPROPERTY 設定及 Actor 狀態而略有不同。

### 9.1 Flood Mesh

| 欄位 | 用途 |
|---|---|
| Water Mesh | 第一個水面 Procedural Mesh Component |
| Water Mesh Chunks | 儲存所有水面 Mesh Chunk |
| Water Material | 水面使用的材質 |

#### Water Material

目前使用：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flood Mesh")
UMaterialInterface* WaterMaterial;
```

用途：

- 指定水面的材質。
- 控制水面顏色。
- 控制透明度。
- 控制粗糙度與金屬度。
- 後續可增加水面視覺效果。

設定方式：

1. 選取 `GeoTIFFReaderActor`。
2. 展開 Details → Flood Mesh。
3. 找到 `Water Material`。
4. 點選材質欄位。
5. 選擇專案中的水面材質。

若尚未指定材質，程式不會自動套用專案自訂的 Water Material。

### 9.2 GDAL

| 欄位 | 用途 |
|---|---|
| GeoTIFF Path | 目前讀取的水深 GeoTIFF 路徑 |
| Raster Width | 水深 Raster 寬度 |
| Raster Height | 水深 Raster 高度 |
| Valid Pixel Count | 有效像素數量 |
| Min Depth | 最小有效水深 |
| Max Depth | 最大有效水深 |
| Mean Depth | 平均有效水深 |
| bRasterLoaded | Raster 是否成功載入 |

#### GeoTIFF Path

目前預設路徑：

```text
C:/Users/jeff2/Desktop/PINN/outputs/minxiong/prediction/depth_012.tif
```

動畫播放時，程式會依照影格編號自動更新此路徑。

例如：

```text
depth_000.tif
depth_001.tif
depth_002.tif
...
depth_020.tif
```

因此，播放動畫時應以 `Prediction Directory` 與 `Current Frame Index` 的實際執行流程為準，而不是將 `GeoTIFF Path` 視為固定讀取檔案。

#### Raster Width / Raster Height

目前水深 GeoTIFF 尺寸：

```text
Raster Width  = 741
Raster Height = 480
```

#### Valid Pixel Count

代表成功讀取且通過 NoData 與數值檢查的像素數量。

#### Min Depth / Max Depth / Mean Depth

分別代表目前影格的：

- 最小有效水深。
- 最大有效水深。
- 平均有效水深。

單位均為公尺。

#### bRasterLoaded

表示目前水深 Raster 是否成功載入。

若為 False，代表目前影格沒有成功完成有效資料載入。

### 9.3 GeoTIFF

| 欄位 | 用途 |
|---|---|
| Terrain GeoTIFF Path | DEM 地形資料路徑 |
| bTerrainLoaded | DEM 是否成功載入 |

#### Terrain GeoTIFF Path

目前預設路徑：

```text
C:/Users/jeff2/processing/Minxiong_DEM_20m.tif
```

此資料提供水面 Mesh 的地形高程基準。

#### bTerrainLoaded

表示 DEM 是否成功載入。

DEM 在動畫開始時載入一次，後續切換水深影格時會重複使用已載入的 DEM 資料，不需要每一幀重新讀取 DEM。

### 9.4 Flood Animation

| 欄位 | 預設值 | 用途 |
|---|---|---|
| Prediction Directory | PINN prediction 資料夾 | 指定 21 個 GeoTIFF 的存放位置 |
| Total Frame Count | 21 | 動畫總影格數 |
| Frame Interval Seconds | 1.0 | 每幀切換間隔 |
| Auto Play | True | 是否在遊戲開始時自動播放 |

#### Prediction Directory

目前設定：

```text
C:/Users/jeff2/Desktop/PINN/outputs/minxiong/prediction
```

資料夾應包含：

```text
prediction/
├── depth_000.tif
├── depth_001.tif
├── depth_002.tif
├── depth_003.tif
├── depth_004.tif
├── depth_005.tif
├── depth_006.tif
├── depth_007.tif
├── depth_008.tif
├── depth_009.tif
├── depth_010.tif
├── depth_011.tif
├── depth_012.tif
├── depth_013.tif
├── depth_014.tif
├── depth_015.tif
├── depth_016.tif
├── depth_017.tif
├── depth_018.tif
├── depth_019.tif
└── depth_020.tif
```

#### Total Frame Count

目前設定：

```text
21
```

代表程式會依序讀取：

```text
depth_000.tif ~ depth_020.tif
```

影格索引從 0 開始，因此 21 個影格的最後一幀編號為 20。

#### Frame Interval Seconds

目前設定：

```text
1.0
```

代表每隔 1 秒切換到下一個影格。

若設定為：

```text
0.5
```

則每 0.5 秒切換一次。

若設定為：

```text
2.0
```

則每 2 秒切換一次。

目前 21 幀採用 1 秒間隔，從第 0 幀開始立即顯示，之後切換 20 次，預計總播放時間約 20 秒。

#### Auto Play

目前設定：

```text
True
```

代表 BeginPlay 時自動開始播放。

若設定為：

```text
False
```

則只載入第 0 幀，不自動啟動動畫 Timer。

注意：目前已實作自動播放與播放結束停止，暫停、繼續及重新播放等互動控制尚未納入目前版本。

---

## 10. PINN 淹水預測資料

### 10.1 資料來源

PINN 輸出資料夾：

```text
C:\Users\jeff2\Desktop\PINN\outputs\minxiong\prediction
```

目前使用：

```text
depth_000.tif ~ depth_020.tif
```

共 21 個時間點。

### 10.2 預測時間設定

| 項目 | 數值 |
|---|---|
| 降雨強度 | 20 mm/hr |
| 降雨持續時間 | 10800 秒 |
| 總時間 | 3 小時 |
| 影格數量 | 21 |
| 時間間隔 | 900 秒 |
| 時間間隔 | 15 分鐘 |

時間點：

```text
0
900
1800
2700
3600
4500
5400
6300
7200
8100
9000
9900
10800
11700
12600
13500
14400
15300
16200
17100
18000
```

注意：以上是 PINN 模型的模擬時間；UE 動畫的 `Frame Interval Seconds` 是畫面播放間隔，兩者不是同一個時間尺度。

### 10.3 水深變化

目前已驗證部分影格的水深統計：

| 影格 | 最大水深（m） | 最大水深（cm） |
|---|---:|---:|
| 000 | 0 | 0 |
| 001 | 0.005332 | 0.533 |
| 005 | 0.026659 | 2.666 |
| 010 | 0.053313 | 5.331 |
| 012 | 0.063973 | 6.397 |
| 020 | 0.063962 | 6.396 |

目前第 12 幀後的最大水深變化已相當小。

因此，在 Unreal Viewport 中可能只會觀察到水面高度略微增加。

這不一定代表動畫沒有正常播放，也可能是 PINN 輸出的水深變化本身較小。

---

## 11. 動態水面動畫執行流程

目前動畫由 `BeginPlay()` 啟動。

### 11.1 初始化階段

執行順序：

1. 呼叫 `Super::BeginPlay()`。
2. 載入 DEM。
3. 確認 DEM 成功載入。
4. 設定目前影格為 0。
5. 載入 `depth_000.tif`。
6. 根據水深資料建立水面 Mesh。
7. 若 Auto Play 為 True，啟動 Timer。

### 11.2 影格切換階段

每次 Timer 觸發：

1. 呼叫 `AdvanceToNextFrame()`。
2. 計算下一個影格編號。
3. 檢查是否已到達最後一幀。
4. 呼叫 `LoadAndGenerateFrame()`。
5. 呼叫 `ClearWaterMesh()` 清除舊水面。
6. 組合新影格的 GeoTIFF 路徑。
7. 呼叫 `LoadGeoTIFF()` 載入新水深。
8. 呼叫 `GenerateFullWaterMesh()`。
9. 依據新 Raster 重新建立水面。
10. 等待下一次 Timer 觸發。

### 11.3 動畫結束

當影格編號到達 20 後：

```cpp
GetWorldTimerManager().ClearTimer(AnimationTimerHandle);
```

停止 Timer，並輸出：

```text
[Flood Animation] Animation completed.
```

目前採用單次播放模式，不會自動循環。

### 11.4 動畫流程圖

```text
BeginPlay
    |
    v
LoadTerrainGeoTIFF
    |
    v
DEM Loaded?
    |
    +---- No ----> Stop
    |
    v
Load Frame 000
    |
    v
Generate Water Mesh
    |
    v
AutoPlay?
    |
    +---- No ----> Show Frame 000
    |
    v
Start Timer
    |
    v
AdvanceToNextFrame
    |
    v
ClearWaterMesh
    |
    v
Load Next GeoTIFF
    |
    v
GenerateFullWaterMesh
    |
    v
Frame Index >= 20?
    |
    +---- No ----> Wait for Timer
    |
    v
Clear Timer
    |
    v
Animation Completed
```

---

## 12. 執行專案與測試

### 12.1 執行前檢查

確認以下項目：

- [ ] Unreal Engine 專案已成功開啟。
- [ ] Level 已建立並儲存。
- [ ] World Outliner 中存在 `GeoTIFFReaderActor`。
- [ ] `Water Material` 已依需求指定。
- [ ] `Terrain GeoTIFF Path` 指向正確 DEM。
- [ ] `Prediction Directory` 指向正確 PINN 輸出資料夾。
- [ ] `Total Frame Count` 設定為 21。
- [ ] `Frame Interval Seconds` 設定為 1.0。
- [ ] `Auto Play` 設定為 True。
- [ ] 21 個 GeoTIFF 檔案均存在。

### 12.2 開始測試

1. 開啟 `Minxiong_Map`。
2. 選取 `GeoTIFFReaderActor`。
3. 確認 Details 設定。
4. 點選上方 Toolbar 的 Play。
5. 觀察 Viewport 中的水面變化。
6. 開啟 Output Log。
7. 確認影格是否依序載入。

### 12.3 Output Log

開啟方式：

```text
Window → Output Log
```

目前程式會輸出類似以下訊息：

```text
========== Flood Animation Started ==========

[Flood Animation] DEM loaded successfully.

[Flood Animation] Loading frame 000

[Flood Animation] Frame 000 completed.

[Flood Animation] Timer started. Interval=1.00 seconds.

[Flood Animation] Advancing to frame 001.

[Flood Animation] Loading frame 001

[Flood Animation] Frame 001 completed.

[Flood Animation] Advancing to frame 002.

[Flood Animation] Loading frame 002
```

最後應出現：

```text
[Flood Animation] Advancing to frame 020.

[Flood Animation] Loading frame 020

[Flood Animation] Animation completed.
```

### 12.4 預期測試結果

| 測試項目 | 預期結果 |
|---|---|
| DEM 載入 | 成功 |
| 第 0 幀 | 不顯示水面 |
| 第 1 幀 | 開始出現水面 |
| 第 1～12 幀 | 水面逐漸增加 |
| 第 13～20 幀 | 水深變化較小 |
| 舊 Mesh 清除 | 切換影格時清除 |
| 新 Mesh 生成 | 成功更新 |
| 動畫播放 | 依 Timer 間隔切換 |
| 最後一幀 | 播放結束並停止 Timer |

---

## 13. 常見問題與除錯

### 13.1 GeoTIFF 找不到

檢查：

1. `Prediction Directory` 是否正確。
2. 檔案名稱是否為 `depth_000.tif` 等三位數格式。
3. 檔案是否真的存在。
4. 路徑是否使用正確的斜線。

PowerShell 檢查：

```powershell
Get-ChildItem `
"C:\Users\jeff2\Desktop\PINN\outputs\minxiong\prediction\depth_*.tif"
```

### 13.2 GDALOpen 失敗

可能原因：

- GDAL DLL 未正確載入。
- GDAL 相依 DLL 不完整。
- GeoTIFF 路徑錯誤。
- GDAL 資料目錄設定不正確。

應檢查 Output Log 中：

```text
[GDAL DEBUG] File Exists
[GDAL DEBUG] GDALOpen Result
```

### 13.3 DEM 載入失敗

檢查：

```text
Terrain GeoTIFF Path
```

確認檔案：

```text
C:\Users\jeff2\processing\Minxiong_DEM_20m.tif
```

並確認 DEM 座標系統為 EPSG:3826。

### 13.4 水面沒有出現

檢查：

1. `bRasterLoaded` 是否為 True。
2. `bTerrainLoaded` 是否為 True。
3. `ValidPixelCount` 是否大於 0。
4. `MaxDepth` 是否大於 0.001 m。
5. `Water Material` 是否正確指定。
6. `GenerateFullWaterMesh()` 是否正常執行。

注意：第 0 幀水深為 0，因此沒有水面是預期結果。

### 13.5 水面高度不正確

檢查：

- GeoTIFF 的 GeoTransform。
- DEM 的 GeoTransform。
- EPSG 座標系統。
- Raster 像素中心轉換。
- DEM 雙線性內插。
- 公尺轉換為 Unreal Engine 公分的計算。

水面高度：

```text
Water Surface Elevation
    = Terrain Elevation + Water Depth
```

### 13.6 水面沒有明顯變化

目前 PINN 輸出的最大水深約為 6.4 cm。

因此水面高度變化可能不明顯。

應先確認：

- Output Log 是否顯示影格依序切換。
- `MaxDepth` 是否隨影格改變。
- 水面 Mesh 是否重新生成。

不應直接修改 PINN 原始水深資料來增加視覺效果。

### 13.7 水面殘留

檢查 `ClearWaterMesh()` 是否正常執行。

此函式負責：

- 清除舊 Mesh Sections。
- 銷毀額外建立的 Mesh Chunk。
- 保留固定的 `WaterMesh` Component。
- 清空 `WaterMeshChunks` 陣列。

若沒有正確清除，可能出現不同影格的水面重疊。

---

## 14. 目前開發狀態

### 14.1 已完成項目

- [x] Unreal Engine 5.8.3 專案建立。
- [x] Visual Studio 2022 C++ 開發環境。
- [x] GDAL 第三方函式庫整合。
- [x] GDAL DLL 載入。
- [x] GeoTIFF 讀取。
- [x] Raster 尺寸與統計資料取得。
- [x] DEM GeoTIFF 讀取。
- [x] DEM 雙線性內插。
- [x] Raster 像素座標轉換。
- [x] 地形高程與水深整合。
- [x] Procedural Mesh 水面生成。
- [x] Water Mesh Chunk 分塊生成。
- [x] 舊水面 Mesh 清除。
- [x] 21 幀 GeoTIFF 讀取流程。
- [x] Timer 動畫切換。
- [x] 動畫播放完成後停止。

### 14.2 後續改善項目

- [ ] Water Material 外觀優化。
- [ ] 水面視覺高度倍率控制。
- [ ] 動畫暫停與繼續。
- [ ] 動畫重新播放。
- [ ] 動畫播放速度控制。
- [ ] 顯示目前模擬時間。
- [ ] 顯示即時最大水深與平均水深。
- [ ] 大型 Mesh 更新效能優化。
- [ ] 與其他 GIS 圖層整合。
- [ ] 與 FastAPI 後端整合。
- [ ] 最終場景與展示畫面整合。

---

## 15. 專案後續開發方向

後續建議依照以下順序進行：

1. 完成 Unreal Engine 地形與淹水範圍的空間對齊。
2. 優化 Water Material。
3. 增加水深視覺倍率控制。
4. 增加動畫暫停、繼續與重新播放功能。
5. 增加模擬時間與水深資訊顯示。
6. 優化大量 Procedural Mesh 的生成與更新效率。
7. 整合道路、建築物、人口與避難所等 GIS 圖層。
8. 整合 FastAPI 與 What-If 模擬資料。
9. 完成智慧城市數位孿生淹水模擬平台的展示與測試。

---

## 16. 重要檔案與路徑整理

| 用途 | 路徑 |
|---|---|
| Unreal 專案 | `C:\Users\jeff2\Documents\Unreal Projects\MyProject\` |
| 專案檔案 | `MyProject.uproject` |
| C++ Header | `Source\MyProject\GeoTIFFReaderActor.h` |
| C++ CPP | `Source\MyProject\GeoTIFFReaderActor.cpp` |
| GDAL 第三方函式庫 | `ThirdParty\GDAL\` |
| DEM | `C:\Users\jeff2\processing\Minxiong_DEM_20m.tif` |
| PINN 預測資料夾 | `C:\Users\jeff2\Desktop\PINN\outputs\minxiong\prediction\` |
| 水深影格 | `depth_000.tif` ～ `depth_020.tif` |

---

## 17. 開發注意事項

1. 修改 C++ Header 後，應確認 Unreal Header Tool 能正常產生程式碼。
2. 修改 C++ CPP 後，應重新編譯並確認 Build 成功。
3. 不要在尚未確認 DEM 載入成功前生成水面。
4. 不要在切換影格時重複載入 DEM。
5. 不要將 PINN 的模擬時間與 UE 動畫播放時間混為一談。
6. 不要將水深單位（公尺）與 UE 世界座標單位（公分）混淆。
7. 不要直接使用不同 Raster 的相同像素索引來取得 DEM 高程。
8. 每次更新水深影格時，必須清除舊 Mesh，避免不同影格重疊。
9. PINN 原始輸出資料應保留，不應直接為了視覺效果修改原始預測值。
10. 若要在其他電腦執行，必須重新確認 GDAL、DEM、PINN 資料路徑及 DLL 相依性。
11. 完成新功能後，應同步更新 README，確保文件與實際程式碼一致。

---

**文件用途：** 記錄 Unreal Engine 專案環境、地理資料、GDAL 整合、C++ 水面生成、21 幀淹水動畫及後續開發流程。

**目前版本：** UE 5.8.3 / GeoTIFF Reader / Procedural Water Mesh / 21-Frame Flood Animation