# HezAudio

Hezaerd Audio is an audio library I made for my own projects, built on top of [OpenAL Soft](https://openal-soft.org/). I've never written an audio library before so I figured I'd give it a shot, since I want a library that's easy to use and has a simple API. I'm not sure if I'll ever finish it, but I'll try my best.

### Build status
![premake-setup](https://github.com/Hezaerd/HezAudio/actions/workflows/setup-premake.yml/badge.svg)


## Currently support
- Formats : `.ogg` & `.mp3`
- 3D spatial playback

## TODO
- [ ] Add support for more formats (`.wav` int priority)
- [ ] Unload audio sources from memory
- [ ] Stream audio files
- [ ] Listener positioning API
- [ ] Effects ??

## Exemple
Check out the [demo](https://github.com/Hezaerd/HezAudio/tree/main/HezAudio-Demo) project for more, but it's super simple to use.

```cpp
// Init the audio engine - bool is for debug mode
Hez::Audio::Init(true);
// Load audio source from file - bool is for whether the
// source should be in 3D space or not
auto source = Hez::Audio::LoadAudioSource("Assets/effect.ogg", true);
// Play the audio source
Hez::Audio::Play(source);
```
and you can set various properties on the audio source as well:
```cpp
source.SetPosition(x, y, z);
source.SetGain(2.0f);
source.SetPitch(1.2f);
source.SetLooping(true);
```


## Dependencies
- [OpenAL Soft](https://openal-soft.org/)
- [minimp3](https://github.com/lieff/minimp3)
- [libogg & Vorbis](https://xiph.org/vorbis/)