#include "ObjectFactory.h"
#include "00_FunctionRegister.h"
#include "ActorAll.h"
#include "UiAll.h"
#include "SoundHelper.h"
#include "ModelHelper.h"
#include "RSRoot_DX11.h"
#include "RSMeshHelper.h"
#include "RSStaticResources.h"

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

    if (sceneConfig["ambient-light"].IsNull() ||
        sceneConfig["ambient-light"].Size() != 4)
    {
        P_LOG(LOG_ERROR,
            "the scene's name in json file %s \
                doesn't has an ambient light data\n",
            _path.c_str());
        delete newNode;
        return nullptr;
    }
    DirectX::XMFLOAT4 ambient =
    {
        sceneConfig["ambient-light"][0].GetFloat(),
        sceneConfig["ambient-light"][1].GetFloat(),
        sceneConfig["ambient-light"][2].GetFloat(),
        sceneConfig["ambient-light"][3].GetFloat()
    };
    newNode->SetCurrentAmbient(ambient);

    CreateSceneAssets(newNode, &sceneConfig);

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

void ObjectFactory::CreateSceneAssets(SceneNode* _node, JsonFile* _json)
{
    JsonNode modelRoot = GetJsonNode(_json, "/model-assets");
    UINT modelSize = modelRoot->Size();
    std::string jsonPath = "";
    if (modelSize)
    {
        std::string meshName = "";
        std::string staticMatName = "copper";
        bool forceMat = false;
        RS_MATERIAL_INFO matInfo = {};
        std::string forceDiffuse = "";
        std::string forceNormal = "";
        std::string loadMode = "";
        RS_SUBMESH_DATA meshData = {};
        for (UINT i = 0; i < modelSize; i++)
        {
            meshName = "";
            staticMatName = "copper";
            forceMat = false;
            matInfo = {};
            forceDiffuse = "";
            forceNormal = "";
            loadMode = "";
            meshData = {};

            jsonPath = "/model-assets/" + std::to_string(i);

            meshName = GetJsonNode(_json,
                jsonPath + "/mesh-name")->GetString();

            JsonNode staMatNode = GetJsonNode(_json,
                jsonPath + "/static-material");
            if (staMatNode && !staMatNode->IsNull())
            {
                staticMatName = staMatNode->GetString();
            }

            JsonNode matInfoNode = GetJsonNode(_json,
                jsonPath + "/material-info");
            if (matInfoNode && !matInfoNode->IsNull())
            {
                JsonNode albedoNode = GetJsonNode(_json,
                    jsonPath + "/material-info/albedo");
                JsonNode fresnelNode = GetJsonNode(_json,
                    jsonPath + "/material-info/fresnel");
                JsonNode shininessNode = GetJsonNode(_json,
                    jsonPath + "/material-info/shininess");
                if (!(albedoNode && fresnelNode && shininessNode))
                {
                    P_LOG(LOG_ERROR,
                        "invalid material info in %s\n",
                        jsonPath.c_str());
                    return;
                }
                DirectX::XMFLOAT4 albedo =
                {
                    GetJsonNode(_json,
                    jsonPath + "/material-info/albedo/0")->GetFloat(),
                    GetJsonNode(_json,
                    jsonPath + "/material-info/albedo/1")->GetFloat(),
                    GetJsonNode(_json,
                    jsonPath + "/material-info/albedo/2")->GetFloat(),
                    GetJsonNode(_json,
                    jsonPath + "/material-info/albedo/3")->GetFloat()
                };
                DirectX::XMFLOAT3 fresnel =
                {
                    GetJsonNode(_json,
                    jsonPath + "/material-info/fresnel/0")->GetFloat(),
                    GetJsonNode(_json,
                    jsonPath + "/material-info/fresnel/1")->GetFloat(),
                    GetJsonNode(_json,
                    jsonPath + "/material-info/fresnel/2")->GetFloat()
                };
                float shininess = GetJsonNode(_json,
                    jsonPath + "/material-info/shininess")->GetFloat();
                matInfo.mDiffuseAlbedo = albedo;
                matInfo.mFresnelR0 = fresnel;
                matInfo.mShininess = shininess;
                forceMat = true;
            }

            JsonNode diffuseNode = GetJsonNode(_json,
                jsonPath + "/force-diffuse");
            if (diffuseNode && !diffuseNode->IsNull())
            {
                forceDiffuse = diffuseNode->GetString();
            }

            JsonNode normalNode = GetJsonNode(_json,
                jsonPath + "/force-normal");
            if (normalNode && !normalNode->IsNull())
            {
                forceNormal = normalNode->GetString();
            }

            loadMode = GetJsonNode(_json,
                jsonPath + "/load-mode")->GetString();

            if (loadMode == "model-file")
            {

            }
            else if (loadMode == "program-box")
            {
                float width = GetJsonNode(_json,
                    jsonPath + "/load-info/box-size/0")->GetFloat();
                float height = GetJsonNode(_json,
                    jsonPath + "/load-info/box-size/1")->GetFloat();
                float depth = GetJsonNode(_json,
                    jsonPath + "/load-info/box-size/2")->GetFloat();
                UINT divide = GetJsonNode(_json,
                    jsonPath + "/load-info/divide")->GetUint();
                meshData = GetRSRoot_DX11_Singleton()->MeshHelper()->
                    GeoGenerate()->CreateBox(width, height, depth, divide,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        GetJsonNode(_json,
                            jsonPath + "/load-info/tex-file")->GetString());
            }
            else if (loadMode == "program-sphere")
            {

            }
            else if (loadMode == "program-geo-sphere")
            {

            }
            else if (loadMode == "program-cylinder")
            {

            }
            else if (loadMode == "program-grid")
            {
                float width = GetJsonNode(_json,
                    jsonPath + "/load-info/grid-size/0")->GetFloat();
                float depth = GetJsonNode(_json,
                    jsonPath + "/load-info/grid-size/1")->GetFloat();
                UINT row = GetJsonNode(_json,
                    jsonPath + "/load-info/row-col-count/0")->GetUint();
                UINT col = GetJsonNode(_json,
                    jsonPath + "/load-info/row-col-count/1")->GetUint();
                meshData = GetRSRoot_DX11_Singleton()->MeshHelper()->
                    GeoGenerate()->CreateGrid(width, depth, row, col,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        GetJsonNode(_json,
                            jsonPath + "/load-info/tex-file")->GetString());
            }
            else
            {
                P_LOG(LOG_ERROR, "invlaid model load mode : %s\n",
                    loadMode.c_str());
                return;
            }

            if (!forceMat)
            {
                meshData.mMaterial = *(GetRSRoot_DX11_Singleton()->
                    StaticResources()->GetStaticMaterial(staticMatName));
            }
            else
            {
                meshData.mMaterial = matInfo;
            }

            if (forceDiffuse != "")
            {
                AddDiffuseTexTo(&meshData, forceDiffuse);
            }
            if (forceNormal != "")
            {
                AddBumpedTexTo(&meshData, forceNormal);
            }
            _node->GetAssetsPool()->
                InsertNewMesh(meshName, meshData, MESH_TYPE::OPACITY);
        }
    }

    JsonNode audioRoot = GetJsonNode(_json, "/audio-assets");
    if (audioRoot && audioRoot->Size())
    {
        for (UINT i = 0; i < audioRoot->Size(); i++)
        {
            JsonNode audio = GetJsonNode(_json,
                "/audio-assets/" + std::to_string(i) + "/audio-name");
            std::string name = audio->GetString();
            audio = GetJsonNode(_json,
                "/audio-assets/" + std::to_string(i) + "/audio-file");
            std::string file = audio->GetString();
            LoadSound(name, file);
        }
    }
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
        ATransformComponent atc(compName, nullptr);

        float pos[3] = { 0.f };
        float ang[3] = { 0.f };
        float sca[3] = { 0.f };
        for (UINT i = 0; i < ARRAYSIZE(pos); i++)
        {
            pos[i] = GetJsonNode(_json,
                _jsonPath + "/init-position/" + std::to_string(i))->GetFloat();
            ang[i] = GetJsonNode(_json,
                _jsonPath + "/init-angle/" + std::to_string(i))->GetFloat();
            sca[i] = GetJsonNode(_json,
                _jsonPath + "/init-scale/" + std::to_string(i))->GetFloat();
        }
        atc.ForcePosition({ pos[0],pos[1],pos[2] });
        atc.ForceRotation({ ang[0],ang[1],ang[2] });
        atc.ForceScaling({ sca[0],sca[1],sca[2] });

        COMP_TYPE type = COMP_TYPE::A_TRANSFORM;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, atc);
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
    else if (compType == "interact")
    {
        AInteractComponent aitc(compName, nullptr);

        std::string initFuncName = GetJsonNode(_json,
            _jsonPath + "/init-func-name")->GetString();
        std::string updateFuncName = GetJsonNode(_json,
            _jsonPath + "/update-func-name")->GetString();
        std::string destoryFuncName = GetJsonNode(_json,
            _jsonPath + "/destory-func-name")->GetString();
        auto foundInit = mActorInteractInitFuncPtrMap.find(initFuncName);
        if (foundInit == mActorInteractInitFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid init func name : %s\n",
                initFuncName.c_str());
            return;
        }
        aitc.SetInitFunction(foundInit->second);
        auto foundUpdate = mActorInteractUpdateFuncPtrMap.find(updateFuncName);
        if (foundUpdate == mActorInteractUpdateFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid update func name : %s\n",
                updateFuncName.c_str());
            return;
        }
        aitc.SetUpdateFunction(foundUpdate->second);
        auto foundDestory = mActorInteractDestoryFuncPtrMap.find(destoryFuncName);
        if (foundDestory == mActorInteractDestoryFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid destory func name : %s\n",
                destoryFuncName.c_str());
            return;
        }
        aitc.SetDestoryFunction(foundDestory->second);

        COMP_TYPE type = COMP_TYPE::A_INTERACT;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aitc);
    }
    else if (compType == "timer")
    {
        ATimerComponent atmc(compName, nullptr);

        UINT timerSize = GetJsonNode(_json, _jsonPath + "/timers")->Size();
        for (UINT i = 0; i < timerSize; i++)
        {
            std::string timerName = GetJsonNode(_json,
                _jsonPath + "/timers/" + std::to_string(i))->GetString();
            atmc.AddTimer(timerName);
        }

        COMP_TYPE type = COMP_TYPE::A_TIMER;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, atmc);
    }
    else if (compType == "collision")
    {
        ACollisionComponent acc(compName, nullptr);

        std::string shapeType = GetJsonNode(_json,
            _jsonPath + "/collision-shape")->GetString();
        UINT valueSize = GetJsonNode(_json,
            _jsonPath + "/collision-size")->Size();
        float value[3] = { 0.f };
        for (UINT i = 0; i < valueSize; i++)
        {
            value[i] = GetJsonNode(_json,
                _jsonPath + "/collision-size/" + std::to_string(i))->GetFloat();
        }
        COLLISION_SHAPE shape = COLLISION_SHAPE::SIZE;
        if (shapeType == "sphere") { shape = COLLISION_SHAPE::SPHERE; }
        else if (shapeType == "box") { shape = COLLISION_SHAPE::BOX; }
        else
        {
            P_LOG(LOG_ERROR, "invlaid collision shape name : %s\n",
                shapeType.c_str());
            return;
        }
        acc.CreateCollisionShape(shape, { value[0],value[1],value[2] });

        COMP_TYPE type = COMP_TYPE::A_COLLISION;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, acc);
    }
    else if (compType == "mesh")
    {
        AMeshComponent amc(compName, nullptr);

        UINT meshSize = GetJsonNode(_json,
            _jsonPath + "/meshes")->Size();
        for (UINT i = 0; i < meshSize; i++)
        {
            std::string meshName = GetJsonNode(_json,
                _jsonPath + "/meshes/" + std::to_string(i) + "/mesh-name")->
                GetString();
            JsonNode offsetNode = GetJsonNode(_json,
                _jsonPath + "/meshes/" + std::to_string(i) + "/mesh-offset");
            DirectX::XMFLOAT3 offset = { 0.f,0.f,0.f };
            if (offsetNode && !offsetNode->IsNull())
            {
                offset =
                {
                    GetJsonNode(_json,
                _jsonPath + "/meshes/" + std::to_string(i) +
                "/mesh-offset/0")->GetFloat(),
                    GetJsonNode(_json,
                _jsonPath + "/meshes/" + std::to_string(i) +
                "/mesh-offset/1")->GetFloat(),
                    GetJsonNode(_json,
                _jsonPath + "/meshes/" + std::to_string(i) +
                "/mesh-offset/2")->GetFloat()
                };
            }
            amc.AddMeshInfo(meshName, offset);
        }

        COMP_TYPE type = COMP_TYPE::A_MESH;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, amc);
    }
}

void ObjectFactory::CreateUiComp(SceneNode* _node, UiObject* _ui,
    JsonFile* _json, std::string _jsonPath)
{

}
