#include "HezAudio.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <thread>
#include <filesystem>

#include "AL/al.h"
#include "AL/alext.h"
#include "alc/alcmain.h"
#include "alhelpers.hpp"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

constexpr const unsigned int KILOBITS = 1024;
constexpr const unsigned int MEGABITS = 1024 * KILOBITS;

namespace Hez
{
	static ALCdevice* sDevice = nullptr;
	static mp3dec_t sMp3Decoder;

	static uint8_t* sAudioScratchBuffer = nullptr;
	static uint32_t sAudioScratchBufferSize = 15 * MEGABITS;

	enum class AudioFileFormat
	{
		None = 0,
		Ogg,
		MP3
	};

	static AudioFileFormat GetAudioFileFormat(const std::string& pFilename)
	{
		std::string extension = std::filesystem::path(pFilename).extension().string();

		if (extension == ".ogg") return AudioFileFormat::Ogg;
		if (extension == ".mp3") return AudioFileFormat::MP3;

		return AudioFileFormat::None;
	}

	static ALenum GetOpenALFormat(uint32_t pChannels)
	{
		switch (pChannels)
		{
		case 1: return AL_FORMAT_MONO16;
		case 2: return AL_FORMAT_STEREO16;
		}

		// Assert
		return 0;
	}

	AudioSource Audio::LoadAudioSourceOgg(const std::string& pFilename)
	{
		FILE* file = fopen(pFilename.c_str(), "rb");

		OggVorbis_File vorbisFile;
		if (ov_open_callbacks(file, &vorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
			std::cout << "Could not open .ogg stream\n";

		vorbis_info* vorbisInfo = ov_info(&vorbisFile, -1);

		auto sampleRate = vorbisInfo->rate;
		auto channels = vorbisInfo->channels;
		auto alFormat = GetOpenALFormat(channels);

		uint64_t samples = ov_pcm_total(&vorbisFile, -1);
		float trackDuration = static_cast<float>(samples) / static_cast<float>(sampleRate);
		uint32_t bufferSize = static_cast<uint32_t>(samples) * channels * sizeof(short); // How much bites per sample ??

#ifdef HEZ_DEBUG
		std::cout << "[HezAudio] - File info " << pFilename << ":";
		std::cout << " Channels: " << channels;
		std::cout << " Sample rate: " << sampleRate;
		std::cout << " Duration: " << trackDuration << " seconds";
		std::cout << " Excepted buffer size: " << bufferSize << " bytes\n";
#endif

		if (sAudioScratchBufferSize < bufferSize)
		{
			sAudioScratchBufferSize = bufferSize;
			delete[] sAudioScratchBuffer;
			sAudioScratchBuffer = new uint8_t[sAudioScratchBufferSize];
		}

		uint8_t* oggBuffer = sAudioScratchBuffer;
		uint8_t* bufferPtr = oggBuffer;
		int endOfFile = 0;

		while (!endOfFile)
		{
			int current_section;

			long length = ov_read(&vorbisFile, reinterpret_cast<char*>(bufferPtr), 4096, 0, 2, 1, &current_section);
			bufferPtr += length;
			if (length == 0)
				endOfFile = 1;
			else if (length < 0)
			{
				if (length == OV_EBADLINK)
				{
					fprintf(stderr, "Corrupt bitstream section! Exiting.\n");
					exit(1);
				}
			}
		}

		uint32_t size = bufferPtr - oggBuffer;

#ifdef HEZ_DEBUG
		std::cout << "[HezAudio] - Actual buffer size: " << size << " bytes\n";
#endif

		ov_clear(&vorbisFile);
		fclose(file);

		ALuint buffer;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, alFormat, oggBuffer, size, sampleRate);

		AudioSource	result = { buffer, true, trackDuration };
		alGenSources(1, &result.mSourceHandle);
		alSourcei(result.mSourceHandle, AL_BUFFER, buffer);

		return result;
	}
}