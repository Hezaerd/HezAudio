#pragma once

#include <iostream>
#include <string>

namespace Hez
{
	class AudioSource
	{
	public:
		~AudioSource();

		static AudioSource LoadFromFile(const std::string& pFile, bool pIsSpatial = false);

		bool IsLoaded() const { return mIsLoaded; }

		void SetPosition(float pX, float pY, float pZ);
		void SetGain(float pGain);
		void SetPitch(float pPitch);
		void SetLooping(bool pLooping);
		void SetSpatial(bool pSpatial);

		std::pair<uint32_t, uint32_t> GetLengthMinutesAndSeconds() const;

	private:
		AudioSource() = default;
		AudioSource(uint32_t pHandle, bool pLoaded, float pLength);

	private:
		uint32_t mBufferHandle = 0;
		uint32_t mSourceHandle = 0;

		bool mIsLoaded = false;
		bool mIsSpatial = false;

		float mTotalDuration = 0;

		// Attributes
		float mPosition[3] = { 0.f, 0.f, 0.f };
		float mGain = 1.f;
		float mPitch = 1.f;
		bool mIsLooping = false;

		friend class Audio;
	};

	class Audio
	{
	public:
		static void Init(bool pDebugLogging);

		static AudioSource LoadAudioSource(const std::string& filename);

		static void Play(const AudioSource& pSource);
		static void Pause(const AudioSource& pSource);
		static void Stop(const AudioSource& pSource);

	private:
		static AudioSource LoadAudioSourceOgg(const std::string& filename);
		static AudioSource LoadAudioSourceMp3(const std::string& filename);
	};
}