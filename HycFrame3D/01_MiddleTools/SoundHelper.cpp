#include "SoundHelper.h"
#include <unordered_map>
#include <Windows.h>
#include "PrintLog.h"

static bool g_SoundSystemHasInited = false;
static IXAudio2* gp_XAudio2 = nullptr;
static IXAudio2MasteringVoice* gp_MasteringVoice = nullptr;
std::unordered_map<std::string, DWORD> g_SoundLengthPool;
std::unordered_map<std::string, BYTE*> g_SoundDataPool;
std::unordered_map<std::string, SOUND_HANDLE> g_SoundPool;

static CRITICAL_SECTION g_SoundLock = {};
#define LOCK EnterCriticalSection(&g_SoundLock)
#define UNLOCK LeaveCriticalSection(&g_SoundLock)

bool SoundHasInited()
{
    return g_SoundSystemHasInited;
}

HRESULT CheckChunk(HANDLE hFile, DWORD format,
    DWORD* pChunkSize, DWORD* pChunkDataPosition)
{
    HRESULT hr = S_OK;
    DWORD dwRead = 0;
    DWORD dwChunkType = 0;
    DWORD dwChunkDataSize = 0;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType = 0;
    DWORD dwBytesRead = 0;
    DWORD dwOffset = 0;

    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) ==
        INVALID_SET_FILE_POINTER)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    while (hr == S_OK)
    {
        if (ReadFile(hFile, &dwChunkType, sizeof(DWORD),
            &dwRead, NULL) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD),
            &dwRead, NULL) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        switch (dwChunkType)
        {
        case 'FFIR':
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (ReadFile(hFile, &dwFileType, sizeof(DWORD),
                &dwRead, NULL) == 0)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            break;

        default:
            if (SetFilePointer(hFile, dwChunkDataSize, NULL,
                FILE_CURRENT) == INVALID_SET_FILE_POINTER)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

        dwOffset += sizeof(DWORD) * 2;
        if (dwChunkType == format)
        {
            *pChunkSize = dwChunkDataSize;
            *pChunkDataPosition = dwOffset;

            return S_OK;
        }

        dwOffset += dwChunkDataSize;
        if (dwBytesRead >= dwRIFFDataSize)
        {
            return S_FALSE;
        }
    }

    return S_OK;
}

HRESULT ReadChunkData(HANDLE hFile, void* pBuffer,
    DWORD dwBuffersize, DWORD dwBufferoffset)
{
    DWORD dwRead;

    if (SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN)
        == INVALID_SET_FILE_POINTER)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL)
        == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

void SplitByRomSymbolSound(const std::string& s,
    std::vector<std::string>& v, const std::string& c)
{
    v.clear();
    std::string::size_type pos1 = 0, pos2 = s.find(c);
    while (std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}

bool InitSound()
{
    InitializeCriticalSection(&g_SoundLock);

    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to init com library in sound\n");
    }

    hr = XAudio2Create(&gp_XAudio2, 0);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to create xaudio2 object\n");

        CoUninitialize();

        return false;
    }

    hr = gp_XAudio2->CreateMasteringVoice(&gp_MasteringVoice);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to create master voice object\n");

        if (gp_XAudio2)
        {
            gp_XAudio2->Release();
            gp_XAudio2 = nullptr;
        }

        CoUninitialize();

        return false;
    }

    g_SoundSystemHasInited = true;

    return true;
}

void UninitSound()
{
    ClearSoundPool();

    gp_MasteringVoice->DestroyVoice();
    gp_MasteringVoice = nullptr;

    if (gp_XAudio2)
    {
        gp_XAudio2->Release();
        gp_XAudio2 = nullptr;
    }

    CoUninitialize();

    g_SoundSystemHasInited = false;

    DeleteCriticalSection(&g_SoundLock);
}

void UpdateSound()
{

}

void ClearSoundPool()
{
    for (auto& sound : g_SoundPool)
    {
        if (sound.second)
        {
            sound.second->Stop(0);
            sound.second->DestroyVoice();
        }
    }
    for (auto& soundData : g_SoundDataPool)
    {
        if (soundData.second)
        {
            delete[] soundData.second;
        }
    }
    g_SoundPool.clear();
    g_SoundDataPool.clear();
    g_SoundLengthPool.clear();
}

void LoadSound(std::string name, LOAD_HANDLE path)
{
    HANDLE hFile;
    DWORD dwChunkSize = 0;
    DWORD dwChunkPosition = 0;
    DWORD dwFiletype;
    WAVEFORMATEXTENSIBLE wfx;
    XAUDIO2_BUFFER buffer;

    LOCK;
    if (g_SoundPool.find(name) != g_SoundPool.end())
    {
        UNLOCK;
        return;
    }
    UNLOCK;

    path = ".\\Assets\\Sounds\\" + path;

    memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
    memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

    hFile = CreateFile(path.c_str(), GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        P_LOG(LOG_ERROR,
            "failed to create sound handle\n");
        return;
    }
    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) ==
        INVALID_SET_FILE_POINTER)
    {
        P_LOG(LOG_ERROR,
            "failed to check sound handle by SetFilePointer\n");
        return;
    }

    HRESULT hr = S_OK;

    hr = CheckChunk(hFile, 'FFIR',
        &dwChunkSize, &dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to check wav sound CheckChunk\n");
        return;
    }
    hr = ReadChunkData(hFile, &dwFiletype,
        sizeof(DWORD), dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to check wav sound by ReadChunkData\n");
        return;
    }
    if (dwFiletype != static_cast<DWORD>('EVAW'))
    {
        P_LOG(LOG_ERROR,
            "failed to check wav sound by dwFiletype\n");
        return;
    }

    hr = CheckChunk(hFile, ' tmf',
        &dwChunkSize, &dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to check wav format by CheckChunk\n");
        return;
    }
    hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to check wav format by ReadChunkData\n");
        return;
    }

    DWORD size = 0;
    hr = CheckChunk(hFile, 'atad', &size, &dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to read wav file by CheckChunk\n");
        return;
    }
    LOCK;
    g_SoundLengthPool.insert(std::make_pair(name, size));
    UNLOCK;
    BYTE* pData = new BYTE[size];
    hr = ReadChunkData(hFile, pData,
        size, dwChunkPosition);
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to read wav file by ReadChunkData\n");
        delete[] pData;
        LOCK;
        g_SoundLengthPool.erase(name);
        UNLOCK;
        return;
    }
    LOCK;
    g_SoundDataPool.insert(std::make_pair(name, pData));
    UNLOCK;

    SOUND_HANDLE soundHandle = nullptr;
    hr = gp_XAudio2->CreateSourceVoice(
        &soundHandle, &(wfx.Format));
    if (FAILED(hr))
    {
        P_LOG(LOG_ERROR,
            "failed to create source wav file\n");
        LOCK;
        g_SoundLengthPool.erase(name);
        delete[] pData;
        g_SoundDataPool.erase(name);
        UNLOCK;
        return;
    }
    soundHandle->SetVolume(0.2f);
    LOCK;
    g_SoundPool.insert(std::make_pair(name, soundHandle));
    UNLOCK;
}

void PlayBGM(std::string soundName)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return;
    }
    UNLOCK;
    auto soundSize = g_SoundLengthPool[soundName];
    auto soundData = g_SoundDataPool[soundName];
    auto sound = g_SoundPool[soundName];

    XAUDIO2_VOICE_STATE xa2state;
    XAUDIO2_BUFFER buffer;

    memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
    buffer.AudioBytes = soundSize;
    buffer.pAudioData = soundData;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    sound->GetState(&xa2state);
    if (xa2state.BuffersQueued != 0)
    {
        sound->Stop(0);
        sound->FlushSourceBuffers();
    }

    sound->SubmitSourceBuffer(&buffer);
    sound->Start(0);
}

void StopBGM(std::string soundName)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return;
    }
    UNLOCK;
    auto sound = g_SoundPool[soundName];
    XAUDIO2_VOICE_STATE xa2state;

    sound->GetState(&xa2state);
    if (xa2state.BuffersQueued != 0)
    {
        sound->Stop(0);
        sound->FlushSourceBuffers();
    }
}

void StopBGM()
{
    XAUDIO2_VOICE_STATE xa2state;

    LOCK;
    for (auto& sound : g_SoundPool)
    {
        sound.second->GetState(&xa2state);
        if (xa2state.BuffersQueued != 0)
        {
            sound.second->Stop(0);
            sound.second->FlushSourceBuffers();
        }
    }
    UNLOCK;
}

void SetVolume(std::string soundName, float volume)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return;
    }
    auto sound = g_SoundPool[soundName];
    UNLOCK;
    sound->SetVolume(volume);
}

void PlaySE(std::string soundName)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return;
    }
    auto soundSize = g_SoundLengthPool[soundName];
    auto soundData = g_SoundDataPool[soundName];
    auto sound = g_SoundPool[soundName];
    UNLOCK;

    XAUDIO2_VOICE_STATE xa2state;
    XAUDIO2_BUFFER buffer;

    memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
    buffer.AudioBytes = soundSize;
    buffer.pAudioData = soundData;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = 0;

    sound->GetState(&xa2state);
    if (xa2state.BuffersQueued != 0)
    {
        sound->Stop(0);
        sound->FlushSourceBuffers();
    }

    sound->SubmitSourceBuffer(&buffer);
    sound->Start(0);
}

SOUND_HANDLE GetSoundHandle(std::string&& soundName)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return nullptr;
    }
    SOUND_HANDLE hnd = g_SoundPool[soundName];
    UNLOCK;
    return hnd;
}

SOUND_HANDLE GetSoundHandle(std::string& soundName)
{
    LOCK;
    if (g_SoundPool.find(soundName) == g_SoundPool.end())
    {
        UNLOCK;
        P_LOG(LOG_ERROR,
            "you haven't loaded this sound : [ %s ]\n",
            soundName.c_str());
        return nullptr;
    }
    SOUND_HANDLE hnd = g_SoundPool[soundName];
    UNLOCK;
    return hnd;
}
