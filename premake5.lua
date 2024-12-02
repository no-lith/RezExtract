workspace "RezExtract"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

project "RezExtract"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    system "Windows"

    pchheader "pch.hpp"
    pchsource "src/pch.cpp"
    
    targetname "RezExtract"

    objdir ( "bin/intermediates/%{cfg.architecture}-%{cfg.buildcfg}" )
    targetdir ( "bin/%{cfg.architecture}-%{cfg.buildcfg}" )

    staticruntime "on"

    files
    {
        "inc/*.hpp",
        "src/*.cpp"
    }


    includedirs
    {
        "inc"
    }

    symbols "On"
    flags { "FatalWarnings", "MultiProcessorCompile" }
    warnings "High"

    filter {"platforms:Win32"}
        architecture "x86"

    filter {"platforms:Win64"}
        architecture "x86_64"
    
    filter {}

    filter "Debug"
        defines "DEBUG"
        editandcontinue "Off"
        flags "NoIncrementalLink"
        sanitize "Address"
        optimize "Off"

    filter "Release"
        defines "NDEBUG"
        optimize "Full"