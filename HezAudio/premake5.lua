project "HezAudio"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("%{wks.location}/build/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.hpp",
        "src/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"AL_LIBTYPE_STATIC"
    }

    includedirs
    {
        "src",
        "vendor/OpenAL-Soft/include",
        "vendor/OpenAL-Soft/src",
        "vendor/OpenAL-Soft/src/common",
        "vendor/libogg/include",
        "vendor/Vorbis/include"
        "vendor/minimp3"
    }

    links
    {
        "OpenAL-Soft",
        "Vorbis"
    }

    warnings "Extra"

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "HEZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "HEZ_RELEASE"
        runtime "Release"
        optimize "on"