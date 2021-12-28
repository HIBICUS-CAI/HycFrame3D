#include "SceneManager.h"
#include "ObjectFactory.h"
#include "SceneNode.h"
#include "ObjectContainer.h"

#include "DevUsage.h"

SceneManager::SceneManager() :
    mObjectFactoryPtr(nullptr), mLoadingScenePtr(nullptr),
    mCurrentScenePtr(nullptr), mNextScenePtr(nullptr),
    mLoadSceneFlg(false), mLoadSceneInfo({ "","" }),
    mSceneSwitchFlg(false) {}

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
    if (mCurrentScenePtr != mLoadingScenePtr)
    {
        mCurrentScenePtr->ReleaseScene();
        delete mCurrentScenePtr;
    }
    ReleaseLoadingScene();
}

void SceneManager::LoadSceneNode(std::string&& _name, std::string&& _path)
{
    mLoadSceneFlg = true;
    mLoadSceneInfo[0] = _name;
    mLoadSceneInfo[1] = _path;
}

void SceneManager::CheckLoadStatus()
{
    if (mSceneSwitchFlg) { mSceneSwitchFlg = false; }

    // TEMP-----------------------
    static bool firstTime = true;
    if (firstTime)
    {
        mNextScenePtr = new SceneNode("dev-usage", this);
        DevUsage(mNextScenePtr);
        firstTime = false;
        mSceneSwitchFlg = true;
    }
    // TEMP-----------------------

    if (mLoadSceneFlg)
    {
        mLoadSceneFlg = false;
        mSceneSwitchFlg = true;

        mCurrentScenePtr->ReleaseScene();
        delete mCurrentScenePtr;
        mCurrentScenePtr = mLoadingScenePtr;
        LoadNextScene();
    }

    if (mCurrentScenePtr == mLoadingScenePtr && mNextScenePtr)
    {
        mSceneSwitchFlg = true;
        mCurrentScenePtr = mNextScenePtr;
        mNextScenePtr = nullptr;
    }
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
    mLoadingScenePtr->ReleaseScene();
    delete mLoadingScenePtr;
}

void SceneManager::LoadNextScene()
{
    // TEMP-------------------------------------------
    if (mLoadSceneInfo[0] == "test1" && mLoadSceneInfo[1] == "test1")
    {
        mNextScenePtr = CreateScene1(this);
        if (mNextScenePtr)
        {
            mLoadSceneInfo = { "","" };
        }
    }
    // TEMP-------------------------------------------
}

bool SceneManager::GetSceneSwitchFlg() const
{
    return mSceneSwitchFlg;
}
