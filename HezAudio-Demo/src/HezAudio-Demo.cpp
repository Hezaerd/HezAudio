#include <HezAudio.hpp>

#include <chrono>
#include <thread>

int main()
{
	// Init the audio engine
	Hez::Audio::Audio::Init(true);

	// Load an audio source
	auto source = Hez::Audio::AudioSource::LoadFromFile("Assets/Cosmic-Cove-Music.mp3", false);
	// Make it loop forever
	source.SetLooping(true);
	// Reduce the volume
	source.SetVolume(1.f);
	// Play the audio source
	Hez::Audio::Audio::Play(source);

	auto frontLeftSource = Hez::Audio::AudioSource::LoadFromFile("Assets/Minecraft Zombie.ogg", true);
	frontLeftSource.SetPosition(-5.f, 0.f, 5.f);

	auto frontRightSource = Hez::Audio::AudioSource::LoadFromFile("Assets/Minecraft Spider.ogg", true);
	frontRightSource.SetPosition(5.f, 0.f, 5.f);

	auto movingSource = Hez::Audio::AudioSource::LoadFromFile("Assets/Minecraft Footsteps.ogg", true);
	movingSource.SetPosition(5.f, 0.f, 5.f);

	int sourceIndex = 0;
	const int sourceCount = 3;
	Hez::Audio::AudioSource* sources[] = { &frontLeftSource, &frontRightSource, &movingSource };

	float xPosition = 5.f;
	float playFrequency = 3.f; // play every 3 seconds
	float timer = playFrequency;

	std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
	while (true)
	{
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta = currentTime - lastTime;
		lastTime = currentTime;

		if (timer < 0.0f)
		{
			timer = playFrequency;
			Hez::Audio::Audio::Play(*sources[sourceIndex++]);
		}

		if (sourceIndex == 3)
		{
			xPosition -= delta.count() * 2.f;
			movingSource.SetPosition(xPosition, 0.f, 5.f);

			sourceIndex = 0;
		}
		else
		{
			xPosition = 5.f;
		}

		timer -= delta.count();

		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(5ms);
	}
}