#include "SoundHelper.h"

#include "PrintLog.h"

#include <unordered_map>
#include <windows.h>

static bool G_SoundSystemHasInited = false;
static IXAudio2 *G_XAudio2 = nullptr;
static IXAudio2MasteringVoice *G_MasteringVoice = nullptr;
static std::unordered_map<std::string, DWORD> G_SoundLengthPool = {};
static std::unordered_map<std::string, BYTE *> G_SoundDataPool = {};
static std::unordered_map<std::string, SOUND_HANDLE> G_SoundPool = {};

static CRITICAL_SECTION G_SoundLock = {};

#define LOCK EnterCriticalSection(&G_SoundLock)
#define UNLOCK LeaveCriticalSection(&G_SoundLock)

bool soundHasInited() { return G_SoundSystemHasInited; }

HRESULT CheckChunk(HANDLE FileHandle,
                   DWORD Format,
                   DWORD *ChunkSizePtr,
                   DWORD *ChunkDataPositionPtr) {
  HRESULT Hr = S_OK;
  DWORD DWRead = 0;
  DWORD DWChunkType = 0;
  DWORD DWChunkDataSize = 0;
  DWORD DWRIFFDataSize = 0;
  DWORD DWFileType = 0;
  DWORD DWBytesRead = 0;
  DWORD DWOffset = 0;

  if (SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN) ==
      INVALID_SET_FILE_POINTER) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  while (Hr == S_OK) {
    if (ReadFile(FileHandle, &DWChunkType, sizeof(DWORD), &DWRead, NULL) == 0) {
      Hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (ReadFile(FileHandle, &DWChunkDataSize, sizeof(DWORD), &DWRead, NULL) ==
        0) {
      Hr = HRESULT_FROM_WIN32(GetLastError());
    }

    switch (DWChunkType) {
    case 'FFIR':
      DWRIFFDataSize = DWChunkDataSize;
      DWChunkDataSize = 4;
      if (ReadFile(FileHandle, &DWFileType, sizeof(DWORD), &DWRead, NULL) ==
          0) {
        Hr = HRESULT_FROM_WIN32(GetLastError());
      }
      break;

    default:
      if (SetFilePointer(FileHandle, DWChunkDataSize, NULL, FILE_CURRENT) ==
          INVALID_SET_FILE_POINTER) {
        return HRESULT_FROM_WIN32(GetLastError());
      }
    }

    DWOffset += sizeof(DWORD) * 2;
    if (DWChunkType == Format) {
      *ChunkSizePtr = DWChunkDataSize;
      *ChunkDataPositionPtr = DWOffset;

      return S_OK;
    }

    DWOffset += DWChunkDataSize;
    if (DWBytesRead >= DWRIFFDataSize) {
      return S_FALSE;
    }
  }

  return S_OK;
}

HRESULT ReadChunkData(HANDLE FileHandle,
                      void *BufferPtr,
                      DWORD DWBuffersize,
                      DWORD DWBufferoffset) {
  DWORD DWRead;

  if (SetFilePointer(FileHandle, DWBufferoffset, NULL, FILE_BEGIN) ==
      INVALID_SET_FILE_POINTER) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  if (ReadFile(FileHandle, BufferPtr, DWBuffersize, &DWRead, NULL) == 0) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  return S_OK;
}

void SplitByRomSymbolSound(const std::string &S,
                           std::vector<std::string> &V,
                           const std::string &C) {
  V.clear();
  std::string::size_type Pos1 = 0, Pos2 = S.find(C);
  while (std::string::npos != Pos2) {
    V.push_back(S.substr(Pos1, Pos2 - Pos1));

    Pos1 = Pos2 + C.size();
    Pos2 = S.find(C, Pos1);
  }
  if (Pos1 != S.length())
    V.push_back(S.substr(Pos1));
}

bool initSound() {
  InitializeCriticalSection(&G_SoundLock);

  HRESULT Hr;

  Hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to init com library in sound");
  }

  Hr = XAudio2Create(&G_XAudio2, 0);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to create xaudio2 object");

    CoUninitialize();

    return false;
  }

  Hr = G_XAudio2->CreateMasteringVoice(&G_MasteringVoice);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to create master voice object");

    if (G_XAudio2) {
      G_XAudio2->Release();
      G_XAudio2 = nullptr;
    }

    CoUninitialize();

    return false;
  }

  G_SoundSystemHasInited = true;

  return true;
}

void uninitSound() {
  clearSoundPool();

  G_MasteringVoice->DestroyVoice();
  G_MasteringVoice = nullptr;

  if (G_XAudio2) {
    G_XAudio2->Release();
    G_XAudio2 = nullptr;
  }

  CoUninitialize();

  G_SoundSystemHasInited = false;

  DeleteCriticalSection(&G_SoundLock);
}

void updateSound() {}

void clearSoundPool() {
  for (auto &Sound : G_SoundPool) {
    if (Sound.second) {
      Sound.second->Stop(0);
      Sound.second->DestroyVoice();
    }
  }
  for (auto &SoundData : G_SoundDataPool) {
    if (SoundData.second) {
      delete[] SoundData.second;
    }
  }
  G_SoundPool.clear();
  G_SoundDataPool.clear();
  G_SoundLengthPool.clear();
}

void loadSound(const std::string &Name, LOAD_HANDLE Path) {
  HANDLE FileHandle;
  DWORD DWChunkSize = 0;
  DWORD DWChunkPosition = 0;
  DWORD DWFiletype = 0;
  WAVEFORMATEXTENSIBLE WFX;
  XAUDIO2_BUFFER Buffer;

  LOCK;
  if (G_SoundPool.find(Name) != G_SoundPool.end()) {
    UNLOCK;
    return;
  }
  UNLOCK;

  Path = ".\\Assets\\Sounds\\" + Path;

  memset(&WFX, 0, sizeof(WAVEFORMATEXTENSIBLE));
  memset(&Buffer, 0, sizeof(XAUDIO2_BUFFER));

  FileHandle = CreateFile(Path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                          OPEN_EXISTING, 0, NULL);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    P_LOG(LOG_ERROR, "failed to create sound handle");
    return;
  }
  if (SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN) ==
      INVALID_SET_FILE_POINTER) {
    P_LOG(LOG_ERROR, "failed to check sound handle by SetFilePointer");
    return;
  }

  HRESULT Hr = S_OK;

  Hr = CheckChunk(FileHandle, 'FFIR', &DWChunkSize, &DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to check wav sound CheckChunk");
    return;
  }
  Hr = ReadChunkData(FileHandle, &DWFiletype, sizeof(DWORD), DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to check wav sound by ReadChunkData");
    return;
  }
  if (DWFiletype != static_cast<DWORD>('EVAW')) {
    P_LOG(LOG_ERROR, "failed to check wav sound by DWFiletype");
    return;
  }

  Hr = CheckChunk(FileHandle, ' tmf', &DWChunkSize, &DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to check wav Format by CheckChunk");
    return;
  }
  Hr = ReadChunkData(FileHandle, &WFX, DWChunkSize, DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to check wav Format by ReadChunkData");
    return;
  }

  DWORD Size = 0;
  Hr = CheckChunk(FileHandle, 'atad', &Size, &DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to read wav file by CheckChunk");
    return;
  }
  LOCK;
  G_SoundLengthPool.insert(std::make_pair(Name, Size));
  UNLOCK;
  BYTE *DataPtr = new BYTE[Size];
  Hr = ReadChunkData(FileHandle, DataPtr, Size, DWChunkPosition);
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to read wav file by ReadChunkData");
    delete[] DataPtr;
    LOCK;
    G_SoundLengthPool.erase(Name);
    UNLOCK;
    return;
  }
  LOCK;
  G_SoundDataPool.insert(std::make_pair(Name, DataPtr));
  UNLOCK;

  SOUND_HANDLE SoundHandle = nullptr;
  Hr = G_XAudio2->CreateSourceVoice(&SoundHandle, &(WFX.Format));
  if (FAILED(Hr)) {
    P_LOG(LOG_ERROR, "failed to create source wav file");
    LOCK;
    G_SoundLengthPool.erase(Name);
    delete[] DataPtr;
    G_SoundDataPool.erase(Name);
    UNLOCK;
    return;
  }
  SoundHandle->SetVolume(0.2f);
  LOCK;
  G_SoundPool.insert(std::make_pair(Name, SoundHandle));
  UNLOCK;
}

void playBGM(const std::string &SoundName) {
  LOCK;
  if (G_SoundPool.find(SoundName) == G_SoundPool.end()) {
    UNLOCK;
    P_LOG(LOG_ERROR, "you haven't loaded this sound : [ {} ]", SoundName);
    return;
  }
  UNLOCK;
  auto SoundSize = G_SoundLengthPool[SoundName];
  auto SoundData = G_SoundDataPool[SoundName];
  auto Sound = G_SoundPool[SoundName];

  XAUDIO2_VOICE_STATE Xa2State;
  XAUDIO2_BUFFER Buffer;

  memset(&Buffer, 0, sizeof(XAUDIO2_BUFFER));
  Buffer.AudioBytes = SoundSize;
  Buffer.pAudioData = SoundData;
  Buffer.Flags = XAUDIO2_END_OF_STREAM;
  Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

  Sound->GetState(&Xa2State);
  if (Xa2State.BuffersQueued != 0) {
    Sound->Stop(0);
    Sound->FlushSourceBuffers();
  }

  Sound->SubmitSourceBuffer(&Buffer);
  Sound->Start(0);
}

void stopBGM(const std::string &SoundName) {
  LOCK;
  if (G_SoundPool.find(SoundName) == G_SoundPool.end()) {
    UNLOCK;
    P_LOG(LOG_ERROR, "you haven't loaded this Sound : [ {} ]", SoundName);
    return;
  }
  UNLOCK;
  auto Sound = G_SoundPool[SoundName];
  XAUDIO2_VOICE_STATE Xa2State;

  Sound->GetState(&Xa2State);
  if (Xa2State.BuffersQueued != 0) {
    Sound->Stop(0);
    Sound->FlushSourceBuffers();
  }
}

void stopBGM() {
  XAUDIO2_VOICE_STATE Xa2State;

  LOCK;
  for (auto &Sound : G_SoundPool) {
    Sound.second->GetState(&Xa2State);
    if (Xa2State.BuffersQueued != 0) {
      Sound.second->Stop(0);
      Sound.second->FlushSourceBuffers();
    }
  }
  UNLOCK;
}

void setVolume(const std::string &SoundName, float Volume) {
  LOCK;
  if (G_SoundPool.find(SoundName) == G_SoundPool.end()) {
    UNLOCK;
    P_LOG(LOG_ERROR, "you haven't loaded this Sound : [ {} ]", SoundName);
    return;
  }
  auto Sound = G_SoundPool[SoundName];
  UNLOCK;
  Sound->SetVolume(Volume);
}

void playSE(const std::string &SoundName) {
  LOCK;
  if (G_SoundPool.find(SoundName) == G_SoundPool.end()) {
    UNLOCK;
    P_LOG(LOG_ERROR, "you haven't loaded this sound : [ {} ]", SoundName);
    return;
  }
  auto SoundSize = G_SoundLengthPool[SoundName];
  auto SoundData = G_SoundDataPool[SoundName];
  auto Sound = G_SoundPool[SoundName];
  UNLOCK;

  XAUDIO2_VOICE_STATE Xa2State;
  XAUDIO2_BUFFER Buffer;

  memset(&Buffer, 0, sizeof(XAUDIO2_BUFFER));
  Buffer.AudioBytes = SoundSize;
  Buffer.pAudioData = SoundData;
  Buffer.Flags = XAUDIO2_END_OF_STREAM;
  Buffer.LoopCount = 0;

  Sound->GetState(&Xa2State);
  if (Xa2State.BuffersQueued != 0) {
    Sound->Stop(0);
    Sound->FlushSourceBuffers();
  }

  Sound->SubmitSourceBuffer(&Buffer);
  Sound->Start(0);
}

SOUND_HANDLE getSoundHandle(const std::string &SoundName) {
  LOCK;
  if (G_SoundPool.find(SoundName) == G_SoundPool.end()) {
    UNLOCK;
    P_LOG(LOG_ERROR, "you haven't loaded this sound : [ {} ]", SoundName);
    return nullptr;
  }
  SOUND_HANDLE Handle = G_SoundPool[SoundName];
  UNLOCK;
  return Handle;
}
