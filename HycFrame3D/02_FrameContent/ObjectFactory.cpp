#include "ObjectFactory.h"
#include "00_FunctionRegister.h"
#include "ActorAll.h"
#include "UiAll.h"

ObjectFactory::ObjectFactory() :mSceneManagerPtr(nullptr),
mActorInputFuncPtrMap({}), mActorInteractInitFuncPtrMap({}),
mActorInteractUpdateFuncPtrMap({}), mActorInteractDestoryFuncPtrMap({}),
mUiInputFuncPtrMap({}), mUiInteractInitFuncPtrMap({}),
mUiInteractUpdateFuncPtrMap({}), mUiInteractDestoryFuncPtrMap({})
{}

ObjectFactory::~ObjectFactory() {}

bool ObjectFactory::StartUp(SceneManager* _sceneManager)
{
    if (!_sceneManager)
    {
        P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
        return false;
    }

    mSceneManagerPtr = _sceneManager;

    RegisterAllFuncPtr(this);

    return true;
}

void ObjectFactory::CleanAndStop()
{

}

SceneNode* ObjectFactory::CreateSceneNode(std::string _name, std::string _path)
{
    SceneNode* newNode = new SceneNode(_name, mSceneManagerPtr);
    if (!newNode)
    {
        P_LOG(LOG_ERROR,
            "failed to alloc a scene node memory name : %s , %s\n",
            _name.c_str(), _path.c_str());
        return nullptr;
    }

    JsonFile sceneConfig = {};
    LoadJsonFile(&sceneConfig, _path);
    if (sceneConfig.HasParseError())
    {
        P_LOG(LOG_ERROR,
            "failed to parse scene config name %s with error code : %d\n",
            _path.c_str(), sceneConfig.GetParseError());
        delete newNode;
        return nullptr;
    }

    {
        std::string sceneName = sceneConfig["scene-name"].GetString();
        if (sceneName != _name)
        {
            P_LOG(LOG_ERROR,
                "the scene's name in json file %s \
                doesn't pair with what has passed\n",
                _path.c_str());
            delete newNode;
            return nullptr;
        }
    }

    if (sceneConfig.HasMember("actor") && !sceneConfig["actor"].IsNull())
    {
        for (unsigned int i = 0; i < sceneConfig["actor"].Size(); i++)
        {
            CreateActorObject(newNode, &sceneConfig,
                "/actor/" + std::to_string(i));
        }
    }
    if (sceneConfig.HasMember("ui") && !sceneConfig["ui"].IsNull())
    {
        for (unsigned int i = 0; i < sceneConfig["ui"].Size(); i++)
        {
            CreateUiObject(newNode, &sceneConfig,
                "/ui/" + std::to_string(i));
        }
    }

    return newNode;
}

void ObjectFactory::CreateActorObject(SceneNode* _node, JsonFile* _json,
    std::string _jsonPath)
{
    JsonNode actorRoot = GetJsonNode(_json, _jsonPath);
    if (!actorRoot)
    {
        P_LOG(LOG_ERROR, "failed to get actor root node : %s\n",
            _jsonPath.c_str());
        return;
    }

    std::string actorName = "";
    {
        JsonNode actorNameNode = GetJsonNode(_json, _jsonPath + "/actor-name");
        if (actorNameNode && actorNameNode->IsString())
        {
            actorName = actorNameNode->GetString();
        }
        else
        {
            P_LOG(LOG_ERROR, "invalid actor name in : %s\n", _jsonPath);
            return;
        }
    }

    ActorObject actor(actorName, *_node);

    JsonNode compsRoot = GetJsonNode(_json, _jsonPath + "/components");
    if (compsRoot)
    {
        for (UINT i = 0; i < compsRoot->Size(); i++)
        {
            std::string compPath = _jsonPath + "/components/" +
                std::to_string(i);
            CreateActorComp(_node, &actor, _json, compPath);
        }
    }

    _node->AddActorObject(actor);
}

void ObjectFactory::CreateUiObject(SceneNode* _node, JsonFile* _json,
    std::string _jsonPath)
{

}

void ObjectFactory::CreateActorComp(SceneNode* _node, ActorObject* _actor,
    JsonFile* _json, std::string _jsonPath)
{
    JsonNode compRoot = GetJsonNode(_json, _jsonPath);
    std::string compType = GetJsonNode(_json, _jsonPath + "/type")->GetString();
    std::string compName = _actor->GetObjectName() + "-" + compType;

    if (compType == "transform")
    {

    }
    else if (compType == "input")
    {
        AInputComponent aic(compName, nullptr);

        std::string inputFuncName = GetJsonNode(_json,
            _jsonPath + "/func-name")->GetString();
        auto found = mActorInputFuncPtrMap.find(inputFuncName);
        if (found == mActorInputFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid input func name : %s\n",
                inputFuncName.c_str());
            return;
        }
        aic.SetInputFunction(found->second);

        COMP_TYPE type = COMP_TYPE::A_INPUT;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aic);
    }
}

void ObjectFactory::CreateUiComp(SceneNode* _node, UiObject* _ui,
    JsonFile* _json, std::string _jsonPath)
{

}
