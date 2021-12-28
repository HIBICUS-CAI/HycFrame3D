#pragma once

#include "Hyc3DCommon.h"
#include <string>
#include <array>

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    bool StartUp(class ObjectFactory* _objectFactory);
    void CleanAndStop();

    void LoadSceneNode(std::string&& _name, std::string&& _path);
    void CheckLoadStatus();

    class ObjectFactory* GetObjectFactory() const;
    class SceneNode* GetCurrentSceneNode() const;

    bool GetSceneSwitchFlg() const;

private:
    bool LoadLoadingScene();
    void ReleaseLoadingScene();

    void LoadNextScene();

private:
    class ObjectFactory* mObjectFactoryPtr;

    class SceneNode* mLoadingScenePtr;
    class SceneNode* mCurrentScenePtr;
    class SceneNode* mNextScenePtr;

    bool mLoadSceneFlg;
    std::array<std::string, 2> mLoadSceneInfo;

    bool mSceneSwitchFlg;
};
