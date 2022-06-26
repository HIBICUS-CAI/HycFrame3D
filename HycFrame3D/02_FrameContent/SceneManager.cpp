#include "SceneManager.h"
#include "ObjectFactory.h"
#include "SceneNode.h"
#include "ObjectContainer.h"
#include <TextUtility.h>
#include <thread>

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

    mLoadingScenePtr = new SceneNode("temp-loading-scene", this);

    if (!mLoadingScenePtr)
    {
        P_LOG(LOG_ERROR, "fail to create temp loading scene\n");
        return false;
    }

    mCurrentScenePtr = mLoadingScenePtr;

    return true;
}

bool SceneManager::DeferedStartUp()
{
    mLoadingScenePtr->ReleaseScene();
    delete mLoadingScenePtr;
    if (!LoadLoadingScene())
    {
        P_LOG(LOG_ERROR, "failed to load loading scene\n");
        return false;
    }

    using namespace Hyc::Text;
    JsonFile entryInfo = {};
    if (!LoadAndParse(entryInfo,
        ".\\Assets\\Configs\\scene-entry-config.json"))
    {
        P_LOG(LOG_ERROR, "entry scene config json error code : %d\n",
            GetParseError(entryInfo));
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
        SceneNode* needRelScene = nullptr;
        if (mCurrentScenePtr != mLoadingScenePtr)
        {
            needRelScene = mCurrentScenePtr;
            mCurrentScenePtr = mLoadingScenePtr;
        }

        std::thread loadThread(&SceneManager::LoadNextScene,
            this, needRelScene);
        loadThread.detach();
    }

    if (mNextScenePtr && (mCurrentScenePtr == mLoadingScenePtr))
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
    mLoadingScenePtr = mObjectFactoryPtr->CreateSceneNode(
        "loading-scene",
        ".\\Assets\\Scenes\\loading-scene.json");
    mCurrentScenePtr = mLoadingScenePtr;

    return (mLoadingScenePtr ? true : false);
}

void SceneManager::ReleaseLoadingScene()
{
    mLoadingScenePtr->ReleaseScene();
    delete mLoadingScenePtr;
}

void SceneManager::LoadNextScene(SceneNode* _relScene)
{
    if (_relScene)
    {
        _relScene->ReleaseScene();
        delete _relScene;
    }

    mNextScenePtr = mObjectFactoryPtr->CreateSceneNode(
        mLoadSceneInfo[0], mLoadSceneInfo[1]);
}

bool SceneManager::GetSceneSwitchFlg() const
{
    return mSceneSwitchFlg;
}
