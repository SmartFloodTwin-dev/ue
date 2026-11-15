
using System.IO;
using UnrealBuildTool;

public class MyProject : ModuleRules
{
    public MyProject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
    {
        "Core",
        "CoreUObject",
        "Engine",
        "InputCore",
        "ProceduralMeshComponent"
    });

        PrivateDependencyModuleNames.AddRange(
            new string[] { }
        );

        // GDAL third-party directory
        string GDALRoot = Path.GetFullPath(
            Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "GDAL")
        );

        string GDALIncludeDir = Path.Combine(GDALRoot, "include");
        string GDALLibDir = Path.Combine(GDALRoot, "lib");
        string GDALBinDir = Path.Combine(GDALRoot, "bin");

        // Header files
        PublicSystemIncludePaths.Add(GDALIncludeDir);

        // Import library
        PublicAdditionalLibraries.Add(
            Path.Combine(GDALLibDir, "gdal.lib")
        );

        // Delay load GDAL DLL
        PublicDelayLoadDLLs.Add("gdal313.dll");

        // Stage GDAL DLLs into Binaries/Win64/GDAL
        foreach (string DllFile in Directory.GetFiles(GDALBinDir, "*.dll"))
        {
            string DllName = Path.GetFileName(DllFile);

            RuntimeDependencies.Add(
                "$(TargetOutputDir)/GDAL/" + DllName,
                DllFile
            );
        }
    }
}
