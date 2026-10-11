#include "SoundHelper.h"

#include "PrintLog.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <vector>

// one source plays one loaded sound, same as one IXAudio2SourceVoice
// per sound in the xaudio2 implementation.
struct SOUND_VOICE {
  ALuint Source = 0;
  ALuint Buffer = 0;
};

static bool G_SoundSystemHasInited = false;
static ALCdevice *G_AudioDevice = nullptr;
static ALCcontext *G_AudioContext = nullptr;
static std::unordered_map<std::string, SOUND_HANDLE> G_SoundPool = {};

static std::recursive_mutex G_SoundLock = {};

#define LOCK std::lock_guard<std::recursive_mutex> SoundLockGuard(G_SoundLock)

namespace {

struct WAVE_DATA {
  uint16_t Channels = 0;
  uint32_t SampleRate = 0;
  uint16_t BitsPerSample = 0;
  std::vector<char> Samples = {};
};

bool readWaveFile(const std::string &Path, WAVE_DATA &OutWave) {
  std::ifstream File(Path, std::ios::binary);
  if (!File) {
    P_LOG(LOG_ERROR, "failed to open sound file : {}", Path);
    return false;
  }

  char RiffHeader[12] = {};
  File.read(RiffHeader, sizeof(RiffHeader));
  if (!File || std::memcmp(RiffHeader, "RIFF", 4) ||
      std::memcmp(RiffHeader + 8, "WAVE", 4)) {
    P_LOG(LOG_ERROR, "failed to check wav sound header : {}", Path);
    return false;
  }

  bool HasFormat = false;
  bool HasData = false;
  uint16_t FormatTag = 0;
  while (File && !(HasFormat && HasData)) {
    char ChunkId[4] = {};
    uint32_t ChunkSize = 0;
    File.read(ChunkId, 4);
    File.read(reinterpret_cast<char *>(&ChunkSize), sizeof(ChunkSize));
    if (!File) {
      break;
    }

    if (!std::memcmp(ChunkId, "fmt ", 4)) {
      std::vector<char> Format(ChunkSize);
      File.read(Format.data(), ChunkSize);
      if (ChunkSize < 16) {
        break;
      }
      std::memcpy(&FormatTag, Format.data(), 2);
      std::memcpy(&OutWave.Channels, Format.data() + 2, 2);
      std::memcpy(&OutWave.SampleRate, Format.data() + 4, 4);
      std::memcpy(&OutWave.BitsPerSample, Format.data() + 14, 2);
      HasFormat = true;
    } else if (!std::memcmp(ChunkId, "data", 4)) {
      OutWave.Samples.resize(ChunkSize);
      File.read(OutWave.Samples.data(), ChunkSize);
      HasData = true;
    } else {
      File.seekg(ChunkSize, std::ios::cur);
    }
    // chunks are word aligned
    if (ChunkSize & 1) {
      File.seekg(1, std::ios::cur);
    }
  }

  // 1 : WAVE_FORMAT_PCM, 0xFFFE : WAVE_FORMAT_EXTENSIBLE
  if (!HasFormat || !HasData || (FormatTag != 1 && FormatTag != 0xFFFE)) {
    P_LOG(LOG_ERROR, "failed to read wav sound chunks : {}", Path);
    return false;
  }

  return true;
}

ALenum getAlFormat(const WAVE_DATA &Wave) {
  if (Wave.Channels == 1 && Wave.BitsPerSample == 8) {
    return AL_FORMAT_MONO8;
  } else if (Wave.Channels == 1 && Wave.BitsPerSample == 16) {
    return AL_FORMAT_MONO16;
  } else if (Wave.Channels == 2 && Wave.BitsPerSample == 8) {
    return AL_FORMAT_STEREO8;
  } else if (Wave.Channels == 2 && Wave.BitsPerSample == 16) {
    return AL_FORMAT_STEREO16;
  }
  return AL_NONE;
}

bool isVoicePlaying(SOUND_HANDLE Voice) {
  if (!Voice->Source) {
    return false;
  }
  ALint State = AL_STOPPED;
  alGetSourcei(Voice->Source, AL_SOURCE_STATE, &State);
  return State == AL_PLAYING || State == AL_PAUSED;
}

void startVoice(SOUND_HANDLE Voice, bool Loop) {
  if (!Voice->Source) {
    return;
  }
  if (isVoicePlaying(Voice)) {
    alSourceStop(Voice->Source);
  }
  alSourcei(Voice->Source, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
  alSourceRewind(Voice->Source);
  alSourcePlay(Voice->Source);
}

void stopVoice(SOUND_HANDLE Voice) {
  if (isVoicePlaying(Voice)) {
    alSourceStop(Voice->Source);
  }
}

SOUND_HANDLE findVoice(const std::string &SoundName) {
  auto Found = G_SoundPool.find(SoundName);
  if (Found == G_SoundPool.end()) {
    P_LOG(LOG_ERROR, "you haven't loaded this sound : [ {} ]", SoundName);
    return nullptr;
  }
  return Found->second;
}

} // namespace

bool soundHasInited() { return G_SoundSystemHasInited; }

bool initSound() {
  LOCK;

  G_AudioDevice = alcOpenDevice(nullptr);
  if (G_AudioDevice) {
    G_AudioContext = alcCreateContext(G_AudioDevice, nullptr);
    if (!G_AudioContext || !alcMakeContextCurrent(G_AudioContext)) {
      P_LOG(LOG_ERROR, "failed to create openal context");
      if (G_AudioContext) {
        alcDestroyContext(G_AudioContext);
        G_AudioContext = nullptr;
      }
      alcCloseDevice(G_AudioDevice);
      G_AudioDevice = nullptr;
      return false;
    }
  } else {
    // machines without any audio output still run the game silently
    P_LOG(LOG_WARNING, "no audio device found, sound will be muted");
  }

  G_SoundSystemHasInited = true;

  return true;
}

void uninitSound() {
  LOCK;

  clearSoundPool();

  if (G_AudioContext) {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(G_AudioContext);
    G_AudioContext = nullptr;
  }
  if (G_AudioDevice) {
    alcCloseDevice(G_AudioDevice);
    G_AudioDevice = nullptr;
  }

  G_SoundSystemHasInited = false;
}

void updateSound() {}

void clearSoundPool() {
  LOCK;

  for (auto &Sound : G_SoundPool) {
    SOUND_HANDLE Voice = Sound.second;
    if (Voice->Source) {
      alSourceStop(Voice->Source);
      alDeleteSources(1, &Voice->Source);
    }
    if (Voice->Buffer) {
      alDeleteBuffers(1, &Voice->Buffer);
    }
    delete Voice;
  }
  G_SoundPool.clear();
}

void loadSound(const std::string &Name, LOAD_HANDLE Path) {
  {
    LOCK;
    if (G_SoundPool.find(Name) != G_SoundPool.end()) {
      return;
    }
  }

  Path = "./Assets/Sounds/" + Path;

  WAVE_DATA Wave = {};
  if (!readWaveFile(Path, Wave)) {
    return;
  }

  ALenum Format = getAlFormat(Wave);
  if (Format == AL_NONE) {
    P_LOG(LOG_ERROR, "unsupported wav format : {} channels {} bits",
          Wave.Channels, Wave.BitsPerSample);
    return;
  }

  LOCK;

  SOUND_HANDLE Voice = new SOUND_VOICE();
  if (G_AudioContext) {
    alGetError();
    alGenBuffers(1, &Voice->Buffer);
    alBufferData(Voice->Buffer, Format, Wave.Samples.data(),
                 static_cast<ALsizei>(Wave.Samples.size()),
                 static_cast<ALsizei>(Wave.SampleRate));
    alGenSources(1, &Voice->Source);
    alSourcei(Voice->Source, AL_BUFFER, static_cast<ALint>(Voice->Buffer));
    alSourcef(Voice->Source, AL_GAIN, 0.2f);
    if (alGetError() != AL_NO_ERROR) {
      P_LOG(LOG_ERROR, "failed to create source wav file");
      if (Voice->Source) {
        alDeleteSources(1, &Voice->Source);
      }
      if (Voice->Buffer) {
        alDeleteBuffers(1, &Voice->Buffer);
      }
      delete Voice;
      return;
    }
  }

  G_SoundPool.insert(std::make_pair(Name, Voice));
}

void playBGM(const std::string &SoundName) {
  LOCK;
  if (auto Voice = findVoice(SoundName)) {
    startVoice(Voice, true);
  }
}

void stopBGM(const std::string &SoundName) {
  LOCK;
  if (auto Voice = findVoice(SoundName)) {
    stopVoice(Voice);
  }
}

void stopBGM() {
  LOCK;
  for (auto &Sound : G_SoundPool) {
    stopVoice(Sound.second);
  }
}

void setVolume(const std::string &SoundName, float Volume) {
  LOCK;
  if (auto Voice = findVoice(SoundName)) {
    if (Voice->Source) {
      alSourcef(Voice->Source, AL_GAIN, Volume);
    }
  }
}

void playSE(const std::string &SoundName) {
  LOCK;
  if (auto Voice = findVoice(SoundName)) {
    startVoice(Voice, false);
  }
}

SOUND_HANDLE getSoundHandle(const std::string &SoundName) {
  LOCK;
  return findVoice(SoundName);
}
