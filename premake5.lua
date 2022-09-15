workspace "RezExtract"
    configurations
    {
        "Debug", "Release"
    }

    platforms { "Win32", "Win64" }

project "RezExtract"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

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

    filter { "platforms:Win32" }
        system "Windows"
        architecture "x86"

    filter { "platforms:Win64" }
        system "Windows"
        architecture "x86_64"

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "Speed"
        symbols "Off"