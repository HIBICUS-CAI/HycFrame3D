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

    using namespace hyc;
    using namespace hyc::text;
    TomlNode entryInfo = {};
    std::string errorMess = "";
    if (!loadTomlAndParse(entryInfo,
        ".\\Assets\\Configs\\scene-entry-config.toml",
        errorMess))
    {
        P_LOG(LOG_ERROR, "failed to parse entry scene config : %s\n",
            errorMess.c_str());
        return false;
    }

    LoadSceneNode(getAs<std::string>(entryInfo["entry-scene"]["name"]),
        getAs<std::string>(entryInfo["entry-scene"]["file"]));

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
    mLoadingScenePtr = mObjectFactoryPtr->createSceneNode(
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

    mNextScenePtr = mObjectFactoryPtr->createSceneNode(
        mLoadSceneInfo[0], mLoadSceneInfo[1]);
}

bool SceneManager::GetSceneSwitchFlg() const
{
    return mSceneSwitchFlg;
}
