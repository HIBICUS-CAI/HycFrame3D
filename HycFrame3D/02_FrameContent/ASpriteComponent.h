#pragma once

#include "ActorComponent.h"
#include <vector>
#include "RSCommon.h"

class ASpriteComponent :public ActorComponent
{
public:
    ASpriteComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ASpriteComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ASpriteComponent();

    ASpriteComponent& operator=(const ASpriteComponent& _source)
    {
        if (this == &_source) { return *this; }
        mGeoPointName = _source.mGeoPointName;
        mTextureName = _source.mTextureName;
        mIsBillboard = _source.mIsBillboard;
        mSize = _source.mSize;
        mTexCoord = _source.mTexCoord;
        mWithAnimation = _source.mWithAnimation;
        mStride = _source.mStride;
        mMaxCut = _source.mMaxCut;
        mCurrentAnimateCut = _source.mCurrentAnimateCut;
        mRepeatFlg = _source.mRepeatFlg;
        mSwitchTime = _source.mSwitchTime;
        mTimeCounter = _source.mTimeCounter;
        ActorComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    bool CreateGeoPointWithTexture(class SceneNode* _scene,
        std::string& _texName);
    bool CreateGeoPointWithTexture(class SceneNode* _scene,
        std::string&& _texName);

    void SetSpriteProperty(DirectX::XMFLOAT2 _size, DirectX::XMFLOAT4 _texCoord,
        bool _isBillboard);

    void SetAnimationProperty(DirectX::XMFLOAT2 _stride,
        UINT _maxCut, bool _repeatFlg, float _switchTime);

private:
    void SyncTransformDataToInstance();

private:
    std::string mGeoPointName;
    std::string mTextureName;
    bool mIsBillboard;

    DirectX::XMFLOAT2 mSize;
    DirectX::XMFLOAT4 mTexCoord;    // origin u & v, offset length & width

    bool mWithAnimation;
    DirectX::XMFLOAT2 mStride;
    UINT mMaxCut;
    UINT mCurrentAnimateCut;
    bool mRepeatFlg;
    float mSwitchTime;
    float mTimeCounter;
};
