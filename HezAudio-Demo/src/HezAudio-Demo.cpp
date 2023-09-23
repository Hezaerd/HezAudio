#include <HezAudio.hpp>

#include <chrono>
#include <thread>

int main()
{
	// Init the audio engine
	Hez::Audio::Init(true);

	// Load an audio source
	auto source = Hez::AudioSource::LoadFromFile("Assets/Cosmic-Cove-Music.mp3", false);
	// Make it loop forever
	source.SetLooping(true);
	// Reduce the volume
	source.SetVolume(0.05f);
	// Play the audio source
	Hez::Audio::Play(source);

	auto backSource = Hez::AudioSource::LoadFromFile("Assets/Minecraft Zombie.ogg", true);
	backSource.SetPosition(-5.f, 0.f, 0.f);
	Hez::Audio::Play(backSource);

	auto movingSource = Hez::AudioSource::LoadFromFile("Assets/Minecraft Footsteps.ogg", true);
	movingSource.SetPosition(5.f, 0.f, 0.f);

	float xPosition = 10.f;
	float playFrequency = movingSource.GetLengthSeconds(); // total duration of the source
	float timer = playFrequency;

	std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();
	while (true)
	{
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta = currentTime - last_time;
		last_time = currentTime;

		if (timer < 0.0f)
		{
			timer = playFrequency;
			Hez::Audio::Play(movingSource);
		}

		if (xPosition < -10.f)
			xPosition = 10.f;

		movingSource.SetPosition(xPosition, 0.f, 0.f);
		xPosition -= 0.1f * delta.count();

		timer -= delta.count();

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5ms);
	}
}