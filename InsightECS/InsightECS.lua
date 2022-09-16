local local_post_build_commands = post_build_commands

project "InsightECS"  
    --kind "SharedLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    { 
        "InsightCore",
        "InsightInput",
    }

    defines
    {
        "IS_EXPORT_ECS_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.optick}",
        "%{IncludeDirs.glm}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp" 
    }

    links
    {
        "InsightCore.lib",
        "InsightInput.lib",
        "glm.lib",
        "tracy.lib",
        "OptickCore.lib",
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    postbuildcommands "%{concat_table(local_post_build_commands)}"

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        links
        {
            "OptickCore.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/debug",
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 
        links
        {
            "OptickCore.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }