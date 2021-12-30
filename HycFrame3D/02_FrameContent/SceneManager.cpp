#include "SceneManager.h"
#include "ObjectFactory.h"
#include "SceneNode.h"
#include "ObjectContainer.h"
#include "JsonHelper.h"

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

bool SceneManager::DeferedStartUp()
{
    JsonFile entryInfo = {};
    LoadJsonFile(&entryInfo, ".\\Assets\\Configs\\scene-entry-config.json");
    if (entryInfo.HasParseError())
    {
        P_LOG(LOG_ERROR, "entry scene config json error code : %d\n",
            entryInfo.GetParseError());
        return false;
    }

    LoadSceneNode(entryInfo["entry-scene-name"].GetString(),
        entryInfo["entry-scene-file"].GetString());

    return true;
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

void SceneManager::LoadSceneNode(std::string&& _name, std::string&& _file)
{
    mLoadSceneFlg = true;
    mLoadSceneInfo[0] = _name;
    mLoadSceneInfo[1] = ".\\Assets\\Scenes\\" + _file;
}

void SceneManager::CheckLoadStatus()
{
    if (mSceneSwitchFlg) { mSceneSwitchFlg = false; }

    if (mLoadSceneFlg)
    {
        mLoadSceneFlg = false;
        mSceneSwitchFlg = true;
        if (mCurrentScenePtr != mLoadingScenePtr)
        {
            mCurrentScenePtr->ReleaseScene();
            delete mCurrentScenePtr;
            mCurrentScenePtr = mLoadingScenePtr;
        }

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
    if (mLoadSceneInfo[0] == "test1" &&
        mLoadSceneInfo[1] == ".\\Assets\\Scenes\\test1")
    {
        mNextScenePtr = CreateScene1(this);
        if (mNextScenePtr)
        {
            mLoadSceneInfo = { "","" };
        }
        return;
    }
    else if (mLoadSceneInfo[0] == "test2" &&
        mLoadSceneInfo[1] == ".\\Assets\\Scenes\\test2")
    {
        mNextScenePtr = CreateScene2(this);
        if (mNextScenePtr)
        {
            mLoadSceneInfo = { "","" };
        }
        return;
    }
    // TEMP-------------------------------------------

    mNextScenePtr = mObjectFactoryPtr->CreateSceneNode(
        mLoadSceneInfo[0], mLoadSceneInfo[1]);
}

bool SceneManager::GetSceneSwitchFlg() const
{
    return mSceneSwitchFlg;
}
