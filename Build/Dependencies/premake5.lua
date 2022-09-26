workspace "Dependencies"
    architecture "x64"
    staticruntime "off"

    configurations
    {
        "Debug",
        "Release",
    }
    platforms 
    { 
        "Win64", 
        "Linux" 
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}/../../bin/" .. outputdir
outputdir_obj = "%{wks.location}/../../bin-int/" .. outputdir
outputdir_debug = "%{wks.location}/../../bin/" .. outputdir
output_deps = "%{wks.location}/../../deps/".. outputdir

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["glfw"] = "%{wks.location}/../../vendor/glfw/include"
IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"

LibDirs = {}
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"
LibDirs["glfw"] ="%{wks.location}/../../Vendor/glfw/lib"

group "Dependices"
        include "../../premakeFiles/assimp.lua"
        include "../../premakeFiles/glfw.lua"
        include "../../premakeFiles/glm.lua"
        include "../../premakeFiles/imgui.lua"
        include "../../premakeFiles/optick.lua"
        include "../../premakeFiles/tracy.lua"
        include "../../premakeFiles/tracyProfiler.lua"
        include "../../premakeFiles/doctest.lua"
        include "../../premakeFiles/meshoptimizer.lua"

