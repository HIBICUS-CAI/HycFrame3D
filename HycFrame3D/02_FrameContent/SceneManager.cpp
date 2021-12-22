#include "SceneManager.h"
#include "ObjectFactory.h"

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
    return true;
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

bool SceneManager::LoadLoadingScene()
{
    // TEMP-------------------
    return true;
    // TEMP-------------------
}

void SceneManager::ReleaseLoadingScene()
{

}

void SceneManager::LoadNextScene()
{

}
