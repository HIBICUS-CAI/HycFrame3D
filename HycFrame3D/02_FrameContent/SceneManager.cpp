#include "SceneManager.h"
#include "ObjectFactory.h"
#include "SceneNode.h"
// TEMP--------------------------

// TEMP--------------------------

SceneManager::SceneManager() :
    mObjectFactoryPtr(nullptr), mLoadingScenePtr(nullptr),
    mCurrentScenePtr(nullptr), mNextScenePtr(nullptr),
    mLoadSceneFlg(false), mLoadSceneInfo({ "","" }) {}

SceneManager::~SceneManager() {}

bool SceneManager::StartUp(ObjectFactory* _objectFactory)
{
    if (!_objectFactory)
    {
        P_LOG(LOG_ERROR, "invalid object factory pointer\n");
        return false;
    }

    mObjectFactoryPtr = _objectFactory;

    return LoadLoadingScene();
}

void SceneManager::CleanAndStop()
{

}

void SceneManager::LoadSceneNode(std::string&& _name, std::string&& _path)
{
    mLoadSceneFlg = true;
    mLoadSceneInfo[0] = _name;
    mLoadSceneInfo[1] = _path;
}

void SceneManager::CheckLoadStatus()
{

}

ObjectFactory* SceneManager::GetObjectFactory() const
{
    return mObjectFactoryPtr;
}

SceneNode* SceneManager::GetCurrentSceneNode() const
{
    return mCurrentScenePtr;
}

bool SceneManager::LoadLoadingScene()
{
    mLoadingScenePtr = new SceneNode("loading-scene", this);
    mCurrentScenePtr = mLoadingScenePtr;

    return (mLoadingScenePtr ? true : false);
}

void SceneManager::ReleaseLoadingScene()
{

}

void SceneManager::LoadNextScene()
{

}
