#pragma once

#include "UiComponent.h"
#include <vector>

class USpriteComponent :public UiComponent
{
public:
    USpriteComponent(std::string&& _compName, class UiObject& _uiOwner);
    USpriteComponent(std::string& _compName, class UiObject& _uiOwner);
    virtual ~USpriteComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    bool BindInstanceToAssetsPool(std::string&& _meshName);
    bool BindInstanceToAssetsPool(std::string& _meshName);

private:
    void SyncTransformDataToInstance();

private:
    std::vector<std::string> mMeshesName;
    std::vector<size_t> mInstancesIndex;
};
