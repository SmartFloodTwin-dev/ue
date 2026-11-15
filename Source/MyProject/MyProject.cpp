
#include "MyProject.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

class FMyProjectModule : public FDefaultGameModuleImpl
{
public:

    virtual void StartupModule() override
    {
#if PLATFORM_WINDOWS

        // 1. Resolve absolute GDAL directory
        const FString ProjectDirectory =
            FPaths::ConvertRelativePathToFull(
                FPaths::ProjectDir()
            );

        const FString GDALRoot =
            FPaths::Combine(
                ProjectDirectory,
                TEXT("ThirdParty/GDAL")
            );

        const FString GDALBinDir =
            FPaths::Combine(GDALRoot, TEXT("bin"));

        const FString GDALDataDir =
            FPaths::Combine(GDALRoot, TEXT("data/gdal"));

        const FString PROJDataDir =
            FPaths::Combine(GDALRoot, TEXT("data/proj"));

        const FString GDALDllPath =
            FPaths::Combine(GDALBinDir, TEXT("gdal313.dll"));

        UE_LOG(LogTemp, Warning,
            TEXT("GDAL absolute directory: %s"),
            *GDALBinDir);

        // 2. Verify directory
        if (!FPaths::DirectoryExists(GDALBinDir))
        {
            UE_LOG(LogTemp, Error,
                TEXT("GDAL directory does not exist: %s"),
                *GDALBinDir);
            return;
        }

        // 3. Set Windows DLL search directory
        if (!SetDllDirectoryW(*GDALBinDir))
        {
            UE_LOG(LogTemp, Error,
                TEXT("SetDllDirectoryW failed. Error: %lu"),
                GetLastError());
            return;
        }

        // 4. Explicitly load GDAL DLL
        HMODULE GDALHandle =
            LoadLibraryW(*GDALDllPath);

        if (GDALHandle == nullptr)
        {
            UE_LOG(LogTemp, Error,
                TEXT("Failed to load gdal313.dll. Error: %lu"),
                GetLastError());

            UE_LOG(LogTemp, Error,
                TEXT("DLL path: %s"),
                *GDALDllPath);

            return;
        }

        UE_LOG(LogTemp, Warning,
            TEXT("GDAL DLL loaded successfully: %s"),
            *GDALDllPath);

        // Keep the module loaded for the lifetime of the process.
        // Do not call FreeLibrary here.

        // 5. Configure GDAL and PROJ data
        FPlatformMisc::SetEnvironmentVar(
            TEXT("GDAL_DATA"),
            *GDALDataDir
        );

        FPlatformMisc::SetEnvironmentVar(
            TEXT("PROJ_DATA"),
            *PROJDataDir
        );

        FPlatformMisc::SetEnvironmentVar(
            TEXT("PROJ_LIB"),
            *PROJDataDir
        );

        UE_LOG(LogTemp, Warning,
            TEXT("GDAL_DATA: %s"),
            *GDALDataDir);

        UE_LOG(LogTemp, Warning,
            TEXT("PROJ_DATA: %s"),
            *PROJDataDir);

#endif
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(
    FMyProjectModule,
    MyProject,
    "MyProject"
);
