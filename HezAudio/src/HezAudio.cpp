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

#ifdef HEZ_DEBUG
bool sDebug = true;
#else
bool sDebug = false;
#endif

#define HEZ_LOG(x) std::cout << "[HezAudio] " << x << std::endl

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
		uint32_t bufferSize = static_cast<unsigned long long>(static_cast<uint32_t>(samples)) * channels * sizeof(short); // How much bites per sample ??

		if (sDebug)
		{
			HEZ_LOG("File info - " << pFilename << ":");
			HEZ_LOG("Channels: " << channels);
			HEZ_LOG("Sample rate: " << sampleRate);
			HEZ_LOG("Excepted Size: " << bufferSize);
		}

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

		if (sDebug)
			HEZ_LOG("Actual buffer size: " << size << "\n");

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

	AudioSource Audio::LoadAudioSourceMp3(const std::string& pFilename)
	{
		mp3dec_file_info_t fileInfo = {};
		int loadResult = mp3dec_load(&sMp3Decoder, pFilename.c_str(), &fileInfo, nullptr, nullptr);
		uint32_t size = fileInfo.samples * sizeof(mp3d_sample_t);

		auto sampleRate = fileInfo.hz;
		auto channels = fileInfo.channels;
		auto alFormat = GetOpenALFormat(channels);
		float trackDuration = size / (fileInfo.avg_bitrate_kbps * static_cast<float>(KILOBITS));

		ALuint buffer;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, alFormat, fileInfo.buffer, size, sampleRate);

		AudioSource	result = { buffer, true, trackDuration };
		alGenSources(1, &result.mSourceHandle);
		alSourcei(result.mSourceHandle, AL_BUFFER, buffer);

		if (sDebug)
		{
			HEZ_LOG("File Info -" << pFilename << ":");
			HEZ_LOG("Channels: " << channels);
			HEZ_LOG("Sample Rate: " << sampleRate);
			HEZ_LOG("Size: " << size << " bytes");

			auto [mins, secs] = result.GetLengthMinutesAndSeconds();
			HEZ_LOG("Length: " << mins << "m" << secs << "s\n");
		}

		if (alGetError() != AL_NO_ERROR)
			std::cout << "Failed to setup sound source" << std::endl;

		return result;
	}

	static void PrintAudioDeviceInfo()
	{
		if (sDebug)
		{
			HEZ_LOG("Audio device Info :");
			HEZ_LOG("Name: " << sDevice->DeviceName);
			HEZ_LOG("Sample Rate: " << sDevice->Frequency);
			HEZ_LOG("Max Sources: " << sDevice->SourcesMax);
			HEZ_LOG("Mono: " << sDevice->NumMonoSources);
			HEZ_LOG("Stereo: " << sDevice->NumStereoSources << "\n");
		}
	}

	void Audio::Init(bool pDebugLoggin)
	{
		if (InitAL(sDevice, nullptr, 0) != 0)
			std::cout << "Audio device error!\n";

		sDebug = pDebugLoggin;

		// Debug Output
		PrintAudioDeviceInfo();

		mp3dec_init(&sMp3Decoder);

		sAudioScratchBuffer = new uint8_t[sAudioScratchBufferSize];

		// Set default listener values
		ALfloat listenerPos[] = { 0.0, 0.0, 0.0 };
		ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
		ALfloat listenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };
		alListenerfv(AL_POSITION, listenerPos);
		alListenerfv(AL_VELOCITY, listenerVel);
		alListenerfv(AL_ORIENTATION, listenerOri);
	}

	AudioSource Audio::LoadAudioSource(const std::string& pFilename)
	{
		auto format = GetAudioFileFormat(pFilename);

		switch (format)
		{
		case AudioFileFormat::Ogg: return LoadAudioSourceOgg(pFilename);
		case AudioFileFormat::MP3: return LoadAudioSourceMp3(pFilename);
			// TODO: Add support for other audio formats
		}

		// Loading failed || unsupported format
		return { 0, false, 0.f };
	}

	void Audio::Play(const AudioSource& pAudioSource)
	{
		alSourcePlay(pAudioSource.mSourceHandle);
	}

	void Audio::Pause(const AudioSource& pAudioSource)
	{
		alSourcePause(pAudioSource.mSourceHandle);
	}

	void Audio::Stop(const AudioSource& pAudioSource)
	{
		alSourceStop(pAudioSource.mSourceHandle);
	}

	AudioSource::AudioSource(uint32_t pHandle, bool pLoaded, float pLength)
		: mBufferHandle(pHandle), mIsLoaded(pLoaded), mTotalDuration(pLength)
	{
	}

	AudioSource::~AudioSource()
	{
		// TODO: free OpenAL audio sources
	}

	void AudioSource::SetPosition(float pX, float pY, float pZ)
	{
		mPosition[0] = pX;
		mPosition[1] = pY;
		mPosition[2] = pZ;

		alSourcefv(mSourceHandle, AL_POSITION, mPosition);
	}

	void AudioSource::SetGain(float pGain)
	{
		mGain = pGain;

		alSourcef(mSourceHandle, AL_GAIN, pGain);
	}

	void AudioSource::SetVolume(float pVolume)
	{
		// Just a wrapper for SetGain
		SetGain(pVolume / 100);
	}

	void AudioSource::SetPitch(float pPitch)
	{
		mPitch = pPitch;

		alSourcef(mSourceHandle, AL_PITCH, pPitch);
	}

	void AudioSource::SetLooping(bool pLooping)
	{
		mIsLooping = pLooping;

		alSourcei(mSourceHandle, AL_LOOPING, pLooping);
	}

	void AudioSource::SetSpatial(bool pSpatial)
	{
		mIsSpatial = pSpatial;

		alSourcei(mSourceHandle, AL_SOURCE_SPATIALIZE_SOFT, pSpatial ? AL_TRUE : AL_FALSE);
		alDistanceModel(AL_LINEAR_DISTANCE);
	}

	std::pair<uint32_t, uint32_t> AudioSource::GetLengthMinutesAndSeconds() const
	{
		return { (uint32_t)(mTotalDuration / 60.f), (uint32_t)mTotalDuration % 60 };
	}

	AudioSource AudioSource::LoadFromFile(const std::string& pFilename, bool pSpatial)
	{
		AudioSource audioSource = Audio::LoadAudioSource(pFilename);
		audioSource.SetSpatial(pSpatial);
		return audioSource;
	}
}