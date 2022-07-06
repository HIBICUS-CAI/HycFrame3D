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
loadSound(const std::string &Name, LOAD_HANDLE Path);

void
playBGM(const std::string &SoundName);

void
stopBGM(const std::string &SoundName);

void
stopBGM();

void
setVolume(const std::string &SoundName, float Volume);

void
playSE(const std::string &SoundName);

SOUND_HANDLE
getSoundHandle(const std::string &SoundName);
