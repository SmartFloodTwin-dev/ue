# ue
# Unreal Engine 專案操作 README

## 1. 開啟 Unreal Engine 專案

1. 找到 Unreal Engine 專案檔案：
   `MyProject.uproject`
2. 雙擊 `MyProject.uproject`。
3. 等待 Unreal Engine 5.8.3 開啟專案。
4. 等待 Shader Compilation 與資產載入完成。

## 2. 建立新的 Level（New Map）

1. 開啟 Unreal Editor 上方選單。
2. 點選 **File → Open Level**。
3. 在開啟的視窗中選擇 **New Map**。
4. 選擇需要的空白地圖或預設地圖類型。
5. 建立新地圖後，確認 Unreal Editor 已切換至新的 Level。
6. 建議立即儲存地圖：

   - 點選 **File → Save Current Level As**
   - 輸入地圖名稱，例如 `Minxiong_Map`
   - 儲存至專案的 `Content` 資料夾。

## 3. 開啟 Content Drawer

1. 點選 Unreal Editor 左下角的 **Content Drawer**。
2. 在 Content Drawer 中瀏覽專案資產。
3. 找到並開啟 **民雄邊界**。
4. 確認民雄鄉邊界相關資產可以正常顯示。
5. 依照後續地形建置流程，將民雄邊界作為場景範圍與空間參考。

## 4. 民雄邊界資料

目前專案使用的地理資料設定：

| 項目 | 設定 |
|---|---|
| 區域 | 嘉義縣民雄鄉 |
| 座標系統 | TWD97 / TM2 zone 121 |
| EPSG | EPSG:3826 |
| DEM 解析度 | 20 m |
| 地形資料格式 | GeoTIFF |
| 邊界資料格式 | GeoPackage / 專案匯入資產 |

## 5. Water 模組重新製作原則

本專案的 Water 相關功能全部重新製作。

**不沿用目前既有的 Water 相關實作與設定。**

重新製作範圍包括：

- Water Mesh
- 水面材質
- 水深資料讀取
- 水面高程計算
- 水面座標轉換
- 動態水面更新
- 水面與地形整合

舊有 Water 相關程式、元件、材質及測試結果不列入新版本的實作依據。

## 6. 專案後續開發順序

1. 開啟 `MyProject.uproject`。
2. 建立新的 Level。
3. 載入民雄邊界資產。
4. 建立民雄鄉地形。
5. 確認地形與邊界的座標及位置。
6. 重新規劃並實作 Water 模組。
7. 整合地形與淹水模擬結果。
8. 完成場景測試與展示。

## 7. 開發環境

| 項目 | 版本 / 設定 |
|---|---|
| Unreal Engine | 5.8.3 |
| IDE | Visual Studio 2022 |
| 地理座標系統 | EPSG:3826 |
| 地形來源 | 民雄 20 m DEM |
| 專案名稱 | MyProject |

