#pragma once

#include "UiComponent.h"
#include <vector>
#include "RSCommon.h"

class USpriteComponent :public UiComponent
{
public:
    USpriteComponent(std::string&& _compName, class UiObject* _uiOwner);
    USpriteComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~USpriteComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    bool CreateSpriteMesh(class SceneNode* _scene,
        DirectX::XMFLOAT4 _offsetColor, std::string& _texName);
    bool CreateSpriteMesh(class SceneNode* _scene,
        DirectX::XMFLOAT4 _offsetColor, std::string&& _texName);

    const DirectX::XMFLOAT4& GetOffsetColor() const;
    void SetOffsetColor(DirectX::XMFLOAT4& _offsetColor);
    void SetOffsetColor(DirectX::XMFLOAT4&& _offsetColor);

private:
    void SyncTransformDataToInstance();

private:
    std::string mMeshesName;

    DirectX::XMFLOAT4 mOffsetColor;
};
