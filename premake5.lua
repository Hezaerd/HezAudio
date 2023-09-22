workspace "HezAudio"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "HezAudio/vendor/OpenAL-Soft"
    include "HezAudio/vendor/libogg"
    include "HezAudio/vendor/Vorbis"
group ""

group "Core"
    include "HezAudio"
group ""