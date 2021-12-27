#pragma once

#include <string>
#include <xaudio2.h>

using SOUND_HANDLE = IXAudio2SourceVoice*;
using LOAD_HANDLE = std::string;

bool InitSound();

void UninitSound();

void UpdateSound();

void ClearSoundPool();

void LoadSound(std::string name, LOAD_HANDLE path);

void PlayBGM(std::string soundName);

void StopBGM(std::string soundName);

void StopBGM();

void SetVolume(std::string soundName, float volume);

void PlaySE(std::string soundName);

SOUND_HANDLE GetSoundHandle(std::string&& soundName);

SOUND_HANDLE GetSoundHandle(std::string& soundName);
