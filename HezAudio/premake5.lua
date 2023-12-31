project "HezAudio"
    location "HezAudio"
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
        "%{IncludeDir.OpenALSoft}",
        "%{IncludeDir.OpenALSoftSrc}",
        "%{IncludeDir.OpenALSoftCommon}",
        "%{IncludeDir.libogg}",
        "%{IncludeDir.Vorbis}",
        "%{IncludeDir.minimp3}"
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