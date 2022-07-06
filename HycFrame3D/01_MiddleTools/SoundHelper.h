#pragma once

#include <string>
#include <xaudio2.h>

using SOUND_HANDLE = IXAudio2SourceVoice *;
using LOAD_HANDLE = std::string;

bool
soundHasInited();

bool
initSound();

void
uninitSound();

void
updateSound();

void
clearSoundPool();

void
loadSound(std::string name, LOAD_HANDLE path);

void
playBGM(std::string soundName);

void
stopBGM(std::string soundName);

void
stopBGM();

void
setVolume(std::string soundName, float volume);

void
playSE(std::string soundName);

SOUND_HANDLE
getSoundHandle(std::string &&soundName);

SOUND_HANDLE
getSoundHandle(std::string &soundName);
