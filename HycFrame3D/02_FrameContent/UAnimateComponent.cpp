#include "UAnimateComponent.h"
#include "UiObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "USpriteComponent.h"
#include <vector>
#include "RSCommon.h"
#include "RSRoot_DX11.h"
#include "RSDevices.h"
#include "RSResourceManager.h"
#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"

UAnimateComponent::UAnimateComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mAnimateMap({}), mCurrentAnimateCut(0), mCurrentAnimate(nullptr),
    mAnimateChangedFlg(false), mTimeCounter(0.f)
{

}

UAnimateComponent::UAnimateComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mAnimateMap({}), mCurrentAnimateCut(0), mCurrentAnimate(nullptr),
    mAnimateChangedFlg(false), mTimeCounter(0.f)
{

}

UAnimateComponent::~UAnimateComponent()
{

}

bool UAnimateComponent::Init()
{
    return true;
}

void UAnimateComponent::Update(Timer& _timer)
{
    if (mAnimateChangedFlg)
    {
        mAnimateChangedFlg = false;
        ResetCurrentAnimate();
        SyncAniInfoToSprite();
    }

    if (mCurrentAnimate)
    {
        if (mTimeCounter > mCurrentAnimate->mSwitchTime)
        {
            mTimeCounter = 0.f;
            ++mCurrentAnimateCut;
            if (mCurrentAnimateCut >= mCurrentAnimate->mMaxCut)
            {
                if (mCurrentAnimate->mRepeatFlg) { mCurrentAnimateCut = 0; }
                else { --mCurrentAnimateCut; }
            }
            SyncAniInfoToSprite();
        }
        mTimeCounter += _timer.floatDeltaTime() / 1000.f;
    }
}

void UAnimateComponent::Destory()
{
    mCurrentAnimate = nullptr;
    for (auto& ani : mAnimateMap) { delete ani.second; }

    mAnimateMap.clear();
}

bool UAnimateComponent::LoadAnimate(std::string _aniName, std::string _aniPath,
    DirectX::XMFLOAT2 _stride, UINT _maxCount,
    bool _repeatFlg, float _switchTime)
{
    std::wstring texPathWStr = L"";
    HRESULT hr = S_OK;
    ID3D11ShaderResourceView* srv = nullptr;
    texPathWStr = std::wstring(_aniPath.begin(), _aniPath.end());
    texPathWStr = L".\\Assets\\Textures\\" + texPathWStr;

    auto resourceManager = getRSDX11RootInstance()->getResourceManager();
    auto ifExist = resourceManager->getMeshSrv(_aniPath);

    if (ifExist) {}
    else if (_aniPath.find(".dds") != std::string::npos ||
        _aniPath.find(".DDS") != std::string::npos)
    {
        hr = DirectX::CreateDDSTextureFromFile(
            getRSDX11RootInstance()->getDevices()->getDevice(),
            texPathWStr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            getRSDX11RootInstance()->getResourceManager()->
                addMeshSrv(_aniPath, srv);
        }
        else
        {
            P_LOG(LOG_ERROR, "fail to load texture : %s\n", _aniPath.c_str());
            return false;
        }
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(
            getRSDX11RootInstance()->getDevices()->getDevice(),
            texPathWStr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            getRSDX11RootInstance()->getResourceManager()->
                addMeshSrv(_aniPath, srv);
        }
        else
        {
            P_LOG(LOG_ERROR, "fail to load texture : %s\n", _aniPath.c_str());
            return false;
        }
    }

    ANIMATE_INFO* ani = new ANIMATE_INFO();
    ani->mTexName = _aniPath;
    ani->mMaxCut = _maxCount;
    ani->mStride = _stride;
    ani->mRepeatFlg = _repeatFlg;
    ani->mSwitchTime = _switchTime;

    mAnimateMap.insert({ _aniName,ani });

    return true;
}

void UAnimateComponent::DeleteAnimate(std::string&& _aniName)
{
    auto found = mAnimateMap.find(_aniName);
    if (found != mAnimateMap.end())
    {
        if (mCurrentAnimate == found->second) { mCurrentAnimate = nullptr; }
        delete found->second;
    }
    else
    {
        P_LOG(LOG_ERROR, "this animate doesnt exist : %s\n", _aniName.c_str());
    }
}

void UAnimateComponent::DeleteAnimate(std::string& _aniName)
{
    auto found = mAnimateMap.find(_aniName);
    if (found != mAnimateMap.end())
    {
        if (mCurrentAnimate == found->second) { mCurrentAnimate = nullptr; }
        delete found->second;
    }
    else
    {
        P_LOG(LOG_ERROR, "this animate doesnt exist : %s\n", _aniName.c_str());
    }
}

void UAnimateComponent::ResetCurrentAnimate()
{
    mCurrentAnimateCut = 0;
    mTimeCounter = 0.f;
}

void UAnimateComponent::ClearCurrentAnimate()
{
    mCurrentAnimate = nullptr;
    mCurrentAnimateCut = 0;
    mTimeCounter = 0.f;
}

void UAnimateComponent::ChangeAnimateTo(std::string&& _aniName)
{
    if (mAnimateMap.find(_aniName) == mAnimateMap.end())
    {
        P_LOG(LOG_ERROR, "cannot find this animation : %s\n", _aniName.c_str());
        return;
    }

    mCurrentAnimate = mAnimateMap[_aniName];
    mAnimateChangedFlg = true;
}

void UAnimateComponent::ChangeAnimateTo(std::string& _aniName)
{
    if (mAnimateMap.find(_aniName) == mAnimateMap.end())
    {
        P_LOG(LOG_ERROR, "cannot find this animation : %s\n", _aniName.c_str());
        return;
    }

    mCurrentAnimate = mAnimateMap[_aniName];
    mAnimateChangedFlg = true;
}

void UAnimateComponent::SyncAniInfoToSprite()
{
    std::string uscName = GetUiOwner()->
        GetComponent<USpriteComponent>()->GetCompName();
    auto mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(uscName);

    mesh->mMeshData.Textures[0] = mCurrentAnimate->mTexName;

    float startX = 0.f;
    float startY = 0.f;
    unsigned int maxX = (int)(1.f / mCurrentAnimate->mStride.x);
    maxX = (((1.f / mCurrentAnimate->mStride.x) - maxX) > 0.5f) ?
        (maxX + 1) : maxX;
    startX = (float)(mCurrentAnimateCut % maxX) * mCurrentAnimate->mStride.x;
    startY = (float)(mCurrentAnimateCut / maxX) * mCurrentAnimate->mStride.y;
    DirectX::XMFLOAT4 uv = { startX, startY,
        startX + mCurrentAnimate->mStride.x,
        startY + mCurrentAnimate->mStride.y };

    for (auto& ins : mesh->mInstanceMap)
    {
        ins.second.CustomizedData2 = uv; break;
    }
}
