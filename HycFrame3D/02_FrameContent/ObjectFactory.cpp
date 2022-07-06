#include "ObjectFactory.h"
#include "00_FunctionRegister.h"
#include "ActorAll.h"
#include "UiAll.h"
#include "SoundHelper.h"
#include "ModelHelper.h"
#include "RSRoot_DX11.h"
#include "RSMeshHelper.h"
#include "RSStaticResources.h"
#include <TextUtility.h>

using namespace hyc::text;

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
    if (!loadJsonAndParse(sceneConfig, _path))
    {
        P_LOG(LOG_ERROR,
            "failed to parse scene config name %s with error code : %d\n",
            _path.c_str(), getJsonParseError(sceneConfig));
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

    if (sceneConfig["ambient-factor"].IsNull() ||
        sceneConfig["ambient-factor"].Size() != 4)
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
        sceneConfig["ambient-factor"][0].GetFloat(),
        sceneConfig["ambient-factor"][1].GetFloat(),
        sceneConfig["ambient-factor"][2].GetFloat(),
        sceneConfig["ambient-factor"][3].GetFloat()
    };
    newNode->SetCurrentAmbientFactor(ambient);

    {
        std::string iblEnvTexName = "";
        std::string iblDiffTexName = "";
        std::string iblSpecTexName = "";

        JsonNode iblEnvNode = getJsonNode(sceneConfig, "/ibl-environment");
        JsonNode iblDiffNode = getJsonNode(sceneConfig, "/ibl-diffuse");
        JsonNode iblSpecNode = getJsonNode(sceneConfig, "/ibl-specular");

        if (iblEnvNode) { iblEnvTexName = iblEnvNode->GetString(); }
        if (iblDiffNode) { iblDiffTexName = iblDiffNode->GetString(); }
        if (iblSpecNode) { iblSpecTexName = iblSpecNode->GetString(); }

        newNode->LoadIBLTexture(iblEnvTexName, iblDiffTexName, iblSpecTexName);
    }

    CreateSceneAssets(newNode, sceneConfig);

    if (sceneConfig.HasMember("actor") && !sceneConfig["actor"].IsNull())
    {
        for (unsigned int i = 0; i < sceneConfig["actor"].Size(); i++)
        {
            CreateActorObject(newNode, sceneConfig,
                "/actor/" + std::to_string(i));
        }
    }
    if (sceneConfig.HasMember("ui") && !sceneConfig["ui"].IsNull())
    {
        for (unsigned int i = 0; i < sceneConfig["ui"].Size(); i++)
        {
            CreateUiObject(newNode, sceneConfig,
                "/ui/" + std::to_string(i));
        }
    }

    return newNode;
}

void ObjectFactory::CreateSceneAssets(SceneNode* _node, JsonFile& _json)
{
    JsonNode modelRoot = getJsonNode(_json, "/model-assets");
    std::string jsonPath = "";
    if (modelRoot && modelRoot->Size())
    {
        UINT modelSize = modelRoot->Size();
        std::string meshName = "";
        RS_MATERIAL_INFO matInfo = {};
        std::string forceDiffuse = "";
        std::string forceNormal = "";
        std::string forceMetal = "";
        std::string forceRough = "";
        std::string forceEmiss = "";
        std::string loadMode = "";
        RS_SUBMESH_DATA meshData = {};
        static SUBMESH_BONES bonesData = {};
        bonesData.clear();
        MESH_ANIMATION_DATA* animationData = nullptr;
        for (UINT i = 0; i < modelSize; i++)
        {
            meshName = "";
            int subIndex = 0;
            matInfo = {};
            forceDiffuse = "";
            forceNormal = "";
            loadMode = "";
            meshData = {};

            jsonPath = "/model-assets/" + std::to_string(i);

            meshName = getJsonNode(_json,
                jsonPath + "/mesh-name")->GetString();

            JsonNode matInfoNode = getJsonNode(_json,
                jsonPath + "/material-info");
            if (matInfoNode && !matInfoNode->IsNull())
            {
                JsonNode majorNode = getJsonNode(_json,
                    jsonPath + "/material-info/major-material");
                JsonNode minorNode = getJsonNode(_json,
                    jsonPath + "/material-info/minor-material");
                JsonNode factorNode = getJsonNode(_json,
                    jsonPath + "/material-info/interpolate-factor");
                auto staticResPtr = getRSDX11RootInstance()->
                    getStaticResources();

                assert(staticResPtr && majorNode && minorNode && factorNode);

                std::string majName = majorNode->GetString();
                std::string minName = minorNode->GetString();
                matInfo.MajorMaterialID = staticResPtr->
                    getStaticMaterialIndex(majName);
                matInfo.MinorMaterialID = staticResPtr->
                    getStaticMaterialIndex(minName);
                matInfo.InterpolateFactor = factorNode->GetFloat();
            }
            else
            {
                P_LOG(LOG_ERROR,
                    "mesh %s doesnt have material info\n",
                    meshName.c_str());
            }

            JsonNode diffuseNode = getJsonNode(_json,
                jsonPath + "/force-diffuse");
            if (diffuseNode && !diffuseNode->IsNull())
            {
                forceDiffuse = diffuseNode->GetString();
            }

            JsonNode normalNode = getJsonNode(_json,
                jsonPath + "/force-normal");
            if (normalNode && !normalNode->IsNull())
            {
                forceNormal = normalNode->GetString();
            }

            JsonNode metalNode = getJsonNode(_json,
                jsonPath + "/force-metallic");
            if (metalNode && !metalNode->IsNull())
            {
                forceMetal = metalNode->GetString();
            }

            JsonNode roughNode = getJsonNode(_json,
                jsonPath + "/force-roughness");
            if (roughNode && !roughNode->IsNull())
            {
                forceRough = roughNode->GetString();
            }

            JsonNode emissNode = getJsonNode(_json,
                jsonPath + "/force-emissive");
            if (emissNode && !emissNode->IsNull())
            {
                forceEmiss = emissNode->GetString();
            }

            loadMode = getJsonNode(_json,
                jsonPath + "/load-mode")->GetString();

            if (loadMode == "model-file")
            {
                std::string fileName = getJsonNode(_json,
                    jsonPath + "/load-info/m-file")->GetString();
                std::string fileType = getJsonNode(_json,
                    jsonPath + "/load-info/m-file-type")->GetString();
                auto subIndexNode = getJsonNode(_json,
                    jsonPath + "/load-info/m-sub-mesh-index");
                if (subIndexNode && !subIndexNode->IsNull())
                {
                    subIndex = subIndexNode->GetInt();
                }
                MODEL_FILE_TYPE type = MODEL_FILE_TYPE::BIN;
                if (fileType == "binary")
                {
                    type = MODEL_FILE_TYPE::BIN;
                }
                else if (fileType == "json")
                {
                    type = MODEL_FILE_TYPE::JSON;
                }
                else
                {
                    P_LOG(LOG_ERROR, "invlaid model file type : %s\n",
                        fileType.c_str());
                    return;
                }
                loadModelFile(fileName, type, subIndex, &meshData,
                    &bonesData, &animationData);
            }
            else if (loadMode == "program-box")
            {
                float width = getJsonNode(_json,
                    jsonPath + "/load-info/b-size/0")->GetFloat();
                float height = getJsonNode(_json,
                    jsonPath + "/load-info/b-size/1")->GetFloat();
                float depth = getJsonNode(_json,
                    jsonPath + "/load-info/b-size/2")->GetFloat();
                UINT divide = getJsonNode(_json,
                    jsonPath + "/load-info/b-divide")->GetUint();
                meshData = getRSDX11RootInstance()->getMeshHelper()->
                    getGeoGenerator()->createBox(width, height, depth, divide,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        getJsonNode(_json,
                            jsonPath + "/load-info/b-tex-file")->GetString());
            }
            else if (loadMode == "program-sphere")
            {
                float radius = getJsonNode(_json,
                    jsonPath + "/load-info/s-radius")->GetFloat();
                UINT slice = getJsonNode(_json,
                    jsonPath + "/load-info/s-slice-stack-count/0")->GetUint();
                UINT stack = getJsonNode(_json,
                    jsonPath + "/load-info/s-slice-stack-count/1")->GetUint();
                meshData = getRSDX11RootInstance()->getMeshHelper()->
                    getGeoGenerator()->createSphere(radius, slice, stack,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        getJsonNode(_json,
                            jsonPath + "/load-info/s-tex-file")->GetString());
            }
            else if (loadMode == "program-geo-sphere")
            {
                float radius = getJsonNode(_json,
                    jsonPath + "/load-info/gs-radius")->GetFloat();
                UINT divide = getJsonNode(_json,
                    jsonPath + "/load-info/gs-divide")->GetUint();
                meshData = getRSDX11RootInstance()->getMeshHelper()->
                    getGeoGenerator()->createGeometrySphere(radius, divide,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        getJsonNode(_json,
                            jsonPath + "/load-info/gs-tex-file")->GetString());
            }
            else if (loadMode == "program-cylinder")
            {
                float topRadius = getJsonNode(_json,
                    jsonPath + "/load-info/c-top-btm-het-size/0")->GetFloat();
                float bottomRadius = getJsonNode(_json,
                    jsonPath + "/load-info/c-top-btm-het-size/1")->GetFloat();
                float height = getJsonNode(_json,
                    jsonPath + "/load-info/c-top-btm-het-size/2")->GetFloat();
                UINT slice = getJsonNode(_json,
                    jsonPath + "/load-info/c-slice-stack-count/0")->GetUint();
                UINT stack = getJsonNode(_json,
                    jsonPath + "/load-info/c-slice-stack-count/1")->GetUint();
                meshData = getRSDX11RootInstance()->getMeshHelper()->
                    getGeoGenerator()->createCylinder(bottomRadius, topRadius,
                        height, slice, stack, LAYOUT_TYPE::NORMAL_TANGENT_TEX,
                        false, {},
                        getJsonNode(_json,
                            jsonPath + "/load-info/c-tex-file")->GetString());
            }
            else if (loadMode == "program-grid")
            {
                float width = getJsonNode(_json,
                    jsonPath + "/load-info/g-size/0")->GetFloat();
                float depth = getJsonNode(_json,
                    jsonPath + "/load-info/g-size/1")->GetFloat();
                UINT row = getJsonNode(_json,
                    jsonPath + "/load-info/g-row-col-count/0")->GetUint();
                UINT col = getJsonNode(_json,
                    jsonPath + "/load-info/g-row-col-count/1")->GetUint();
                meshData = getRSDX11RootInstance()->getMeshHelper()->
                    getGeoGenerator()->createGrid(width, depth, row, col,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        getJsonNode(_json,
                            jsonPath + "/load-info/g-tex-file")->GetString());
            }
            else
            {
                P_LOG(LOG_ERROR, "invlaid model load mode : %s\n",
                    loadMode.c_str());
                return;
            }

            meshData.Material = matInfo;

            if (forceDiffuse != "")
            {
                addTextureToSubMesh(&meshData, forceDiffuse,
                    MESH_TEXTURE_TYPE::ALBEDO);
            }
            if (forceNormal != "")
            {
                addTextureToSubMesh(&meshData, forceNormal,
                    MESH_TEXTURE_TYPE::NORMAL);
            }
            if (forceMetal != "")
            {
                addTextureToSubMesh(&meshData, forceMetal,
                    MESH_TEXTURE_TYPE::METALLIC);
            }
            if (forceRough != "")
            {
                addTextureToSubMesh(&meshData, forceRough,
                    MESH_TEXTURE_TYPE::ROUGHNESS);
            }
            if (forceEmiss != "")
            {
                addTextureToSubMesh(&meshData, forceEmiss,
                    MESH_TEXTURE_TYPE::EMISSIVE);
            }

            if (!meshData.Textures.size())
            {
                P_LOG(LOG_ERROR, "invlaid model without diffuse : %s\n",
                    meshName.c_str());
                getRSDX11RootInstance()->getMeshHelper()->
                    releaseSubMesh(meshData);
                return;
            }

            _node->GetAssetsPool()->
                InsertNewIndexedMesh(meshName, meshData,
                    MESH_TYPE::OPACITY, subIndex,
                    &bonesData, animationData);
        }
    }

    JsonNode audioRoot = getJsonNode(_json, "/audio-assets");
    if (audioRoot && audioRoot->Size())
    {
        for (UINT i = 0; i < audioRoot->Size(); i++)
        {
            JsonNode audio = getJsonNode(_json,
                "/audio-assets/" + std::to_string(i) + "/audio-name");
            std::string name = audio->GetString();
            audio = getJsonNode(_json,
                "/audio-assets/" + std::to_string(i) + "/audio-file");
            std::string file = audio->GetString();
            loadSound(name, file);
            _node->GetAssetsPool()->InsertNewSound(name);
        }
    }
}

void ObjectFactory::CreateActorObject(SceneNode* _node, JsonFile& _json,
    std::string _jsonPath)
{
    JsonNode actorRoot = getJsonNode(_json, _jsonPath);
    if (!actorRoot)
    {
        P_LOG(LOG_ERROR, "failed to get actor root node : %s\n",
            _jsonPath.c_str());
        return;
    }

    std::string actorName = "";
    {
        JsonNode actorNameNode = getJsonNode(_json, _jsonPath + "/actor-name");
        if (actorNameNode && actorNameNode->IsString())
        {
            actorName = actorNameNode->GetString();
        }
        else
        {
            P_LOG(LOG_ERROR, "invalid actor name in : %s\n", _jsonPath.c_str());
            return;
        }
    }

    ActorObject actor(actorName, *_node);

    JsonNode compsRoot = getJsonNode(_json, _jsonPath + "/components");
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

void ObjectFactory::CreateUiObject(SceneNode* _node, JsonFile& _json,
    std::string _jsonPath)
{
    JsonNode uiRoot = getJsonNode(_json, _jsonPath);
    if (!uiRoot)
    {
        P_LOG(LOG_ERROR, "failed to get ui root node : %s\n",
            _jsonPath.c_str());
        return;
    }

    std::string uiName = "";
    {
        JsonNode uiNameNode = getJsonNode(_json, _jsonPath + "/ui-name");
        if (uiNameNode && uiNameNode->IsString())
        {
            uiName = uiNameNode->GetString();
        }
        else
        {
            P_LOG(LOG_ERROR, "invalid ui name in : %s\n", _jsonPath.c_str());
            return;
        }
    }

    UiObject ui(uiName, *_node);

    JsonNode compsRoot = getJsonNode(_json, _jsonPath + "/components");
    if (compsRoot)
    {
        for (UINT i = 0; i < compsRoot->Size(); i++)
        {
            std::string compPath = _jsonPath + "/components/" +
                std::to_string(i);
            CreateUiComp(_node, &ui, _json, compPath);
        }
    }

    _node->AddUiObject(ui);
}

void ObjectFactory::CreateActorComp(SceneNode* _node, ActorObject* _actor,
    JsonFile& _json, std::string _jsonPath)
{
    std::string compType = getJsonNode(_json, _jsonPath + "/type")->GetString();
    std::string compName = _actor->GetObjectName() + "-" + compType;

    if (compType == "transform")
    {
        ATransformComponent atc(compName, nullptr);

        float pos[3] = { 0.f };
        float ang[3] = { 0.f };
        float sca[3] = { 0.f };
        for (UINT i = 0; i < ARRAYSIZE(pos); i++)
        {
            pos[i] = getJsonNode(_json,
                _jsonPath + "/atc-init-position/" + std::to_string(i))->GetFloat();
            ang[i] = getJsonNode(_json,
                _jsonPath + "/atc-init-angle/" + std::to_string(i))->GetFloat();
            sca[i] = getJsonNode(_json,
                _jsonPath + "/atc-init-scale/" + std::to_string(i))->GetFloat();
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

        std::string inputFuncName = getJsonNode(_json,
            _jsonPath + "/aic-func-name")->GetString();
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

        std::string initFuncName = getJsonNode(_json,
            _jsonPath + "/aitc-init-func-name")->GetString();
        std::string updateFuncName = getJsonNode(_json,
            _jsonPath + "/aitc-update-func-name")->GetString();
        std::string destoryFuncName = getJsonNode(_json,
            _jsonPath + "/aitc-destory-func-name")->GetString();
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

        UINT timerSize = getJsonNode(_json, _jsonPath + "/atmc-timers")->Size();
        for (UINT i = 0; i < timerSize; i++)
        {
            std::string timerName = getJsonNode(_json,
                _jsonPath + "/atmc-timers/" + std::to_string(i))->GetString();
            atmc.AddTimer(timerName);
        }

        COMP_TYPE type = COMP_TYPE::A_TIMER;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, atmc);
    }
    else if (compType == "collision")
    {
        ACollisionComponent acc(compName, nullptr);

        std::string shapeType = getJsonNode(_json,
            _jsonPath + "/acc-collision-shape")->GetString();
        UINT valueSize = getJsonNode(_json,
            _jsonPath + "/acc-collision-size")->Size();
        float value[3] = { 0.f };
        for (UINT i = 0; i < valueSize; i++)
        {
            value[i] = getJsonNode(_json,
                _jsonPath + "/acc-collision-size/" + std::to_string(i))->GetFloat();
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

        UINT meshSize = getJsonNode(_json,
            _jsonPath + "/amc-meshes")->Size();
        for (UINT i = 0; i < meshSize; i++)
        {
            std::string meshName = getJsonNode(_json,
                _jsonPath + "/amc-meshes/" + std::to_string(i) + "/mesh-name")->
                GetString();
            JsonNode offsetNode = getJsonNode(_json,
                _jsonPath + "/amc-meshes/" + std::to_string(i) + "/mesh-offset");
            DirectX::XMFLOAT3 offset = { 0.f,0.f,0.f };
            if (offsetNode && !offsetNode->IsNull())
            {
                offset =
                {
                    getJsonNode(_json,
                _jsonPath + "/amc-meshes/" + std::to_string(i) +
                "/mesh-offset/0")->GetFloat(),
                    getJsonNode(_json,
                _jsonPath + "/amc-meshes/" + std::to_string(i) +
                "/mesh-offset/1")->GetFloat(),
                    getJsonNode(_json,
                _jsonPath + "/amc-meshes/" + std::to_string(i) +
                "/mesh-offset/2")->GetFloat()
                };
            }
            amc.AddMeshInfo(meshName, offset);
        }

        JsonNode intensityNode = getJsonNode(_json,
            _jsonPath + "/amc-emissive-intensity");
        if (intensityNode && intensityNode->IsFloat())
        {
            amc.SetEmissiveIntensity(GetAs<float>(intensityNode));
        }

        COMP_TYPE type = COMP_TYPE::A_MESH;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, amc);
    }
    else if (compType == "light")
    {
        ALightComponent alc(compName, nullptr);

        std::string lightTypeStr = getJsonNode(_json,
            _jsonPath + "/alc-light-type")->GetString();
        LIGHT_TYPE lightType = LIGHT_TYPE::POINT;
        if (lightTypeStr == "direct")
        {
            lightType = LIGHT_TYPE::DIRECT;
        }
        else if (lightTypeStr == "point")
        {
            lightType = LIGHT_TYPE::POINT;
        }
        else if (lightTypeStr == "spot")
        {
            lightType = LIGHT_TYPE::SPOT;
        }
        else
        {
            P_LOG(LOG_ERROR, "invlaid light type : %s\n", lightTypeStr.c_str());
            return;
        }

        bool bloomFlag = getJsonNode(_json,
            _jsonPath + "/alc-with-bloom")->GetBool();
        bool shadowFlag = false;
        if (lightType == LIGHT_TYPE::DIRECT)
        {
            shadowFlag = getJsonNode(_json,
                _jsonPath + "/alc-with-shadow")->GetBool();
        }

        LIGHT_INFO li = {};
        CAM_INFO ci = {};
        li.Type = lightType;
        li.Position = { 0.f,0.f,0.f };
        li.ShadowFlag = shadowFlag;
        li.Direction.x = getJsonNode(_json,
            _jsonPath + "/alc-direction/0")->GetFloat();
        li.Direction.y = getJsonNode(_json,
            _jsonPath + "/alc-direction/1")->GetFloat();
        li.Direction.z = getJsonNode(_json,
            _jsonPath + "/alc-direction/2")->GetFloat();
        li.Intensity = getJsonNode(_json,
            _jsonPath + "/alc-intensity")->GetFloat();
        li.Albedo.x = getJsonNode(_json,
            _jsonPath + "/alc-albedo/0")->GetFloat();
        li.Albedo.y = getJsonNode(_json,
            _jsonPath + "/alc-albedo/1")->GetFloat();
        li.Albedo.z = getJsonNode(_json,
            _jsonPath + "/alc-albedo/2")->GetFloat();
        li.FalloffStart = getJsonNode(_json,
            _jsonPath + "/alc-fall-off-start-end/0")->GetFloat();
        li.FalloffEnd = getJsonNode(_json,
            _jsonPath + "/alc-fall-off-start-end/1")->GetFloat();
        li.SpotPower = getJsonNode(_json,
            _jsonPath + "/alc-spot-power")->GetFloat();
        if (getJsonNode(_json, _jsonPath + "/alc-cam-up-vec"))
        {
            ci.Type = LENS_TYPE::ORTHOGRAPHIC;
            ci.Position = li.Position;
            ci.LookAtVector = li.Direction;
            ci.NearFarZ = { 0.f,1000.f };
            ci.PerspFovYRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
            ci.OrthoWidthHeight = { 128.f * 9.5f,72.f * 9.5f };
            ci.UpVector.x = getJsonNode(_json,
                _jsonPath + "/alc-cam-up-vec/0")->GetFloat();
            ci.UpVector.y = getJsonNode(_json,
                _jsonPath + "/alc-cam-up-vec/1")->GetFloat();
            ci.UpVector.z = getJsonNode(_json,
                _jsonPath + "/alc-cam-up-vec/2")->GetFloat();
        }

        alc.AddLight(li, bloomFlag, shadowFlag, ci);

        COMP_TYPE type = COMP_TYPE::A_LIGHT;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, alc);
    }
    else if (compType == "audio")
    {
        AAudioComponent aauc(compName, nullptr);

        UINT audioSize = getJsonNode(_json, _jsonPath + "/aauc-sounds")->Size();
        for (UINT i = 0; i < audioSize; i++)
        {
            std::string soundName = getJsonNode(_json,
                _jsonPath + "/aauc-sounds/" + std::to_string(i))->GetString();
            aauc.AddAudio(soundName, *_node);
        }

        COMP_TYPE type = COMP_TYPE::A_AUDIO;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aauc);
    }
    else if (compType == "particle")
    {
        AParticleComponent apc(compName, nullptr);

        std::string ptcTexName = getJsonNode(_json,
            _jsonPath + "/apc-texture-name")->GetString();
        PARTICLE_TEXTURE ptcTex = PARTICLE_TEXTURE::SIZE;
        if (ptcTexName == "circle")
        {
            ptcTex = PARTICLE_TEXTURE::WHITE_CIRCLE;
        }
        else if (ptcTexName == "smoke")
        {
            ptcTex = PARTICLE_TEXTURE::WHITE_SMOKE;
        }
        else
        {
            P_LOG(LOG_ERROR, "invlaid particle texture type : %s\n",
                ptcTexName.c_str());
            return;
        }

        float emitPreSec = getJsonNode(_json,
            _jsonPath + "/apc-emit-per-second")->GetFloat();
        DirectX::XMFLOAT3 velocity =
        {
            getJsonNode(_json,
            _jsonPath + "/apc-velocity/0")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-velocity/1")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-velocity/2")->GetFloat()
        };
        DirectX::XMFLOAT3 posVariance =
        {
            getJsonNode(_json,
            _jsonPath + "/apc-pos-variance/0")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-pos-variance/1")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-pos-variance/2")->GetFloat()
        };
        float velVariance = getJsonNode(_json,
            _jsonPath + "/apc-vel-variance")->GetFloat();
        DirectX::XMFLOAT3 acceleration =
        {
            getJsonNode(_json,
            _jsonPath + "/apc-acceleration/0")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-acceleration/1")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-acceleration/2")->GetFloat()
        };
        float mass = getJsonNode(_json,
            _jsonPath + "/apc-particle-mass")->GetFloat();
        float lifeSpan = getJsonNode(_json,
            _jsonPath + "/apc-life-span")->GetFloat();
        float startSize = getJsonNode(_json,
            _jsonPath + "/apc-start-end-size/0")->GetFloat();
        float endSize = getJsonNode(_json,
            _jsonPath + "/apc-start-end-size/1")->GetFloat();
        DirectX::XMFLOAT4 startColor =
        {
            getJsonNode(_json,
            _jsonPath + "/apc-start-color/0")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-start-color/1")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-start-color/2")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-start-color/3")->GetFloat()
        };
        DirectX::XMFLOAT4 endColor =
        {
            getJsonNode(_json,
            _jsonPath + "/apc-end-color/0")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-end-color/1")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-end-color/2")->GetFloat(),
            getJsonNode(_json,
            _jsonPath + "/apc-end-color/3")->GetFloat()
        };
        bool streakFlg = getJsonNode(_json,
            _jsonPath + "/apc-streak-flag")->GetBool();

        PARTICLE_EMITTER_INFO pei = {};
        pei.EmitNumPerSecond = emitPreSec;
        pei.Velocity = velocity;
        pei.PosVariance = posVariance;
        pei.VelVariance = velVariance;
        pei.Acceleration = acceleration;
        pei.ParticleMass = mass;
        pei.LifeSpan = lifeSpan;
        pei.StartSize = startSize;
        pei.EndSize = endSize;
        pei.StartColor = startColor;
        pei.EndColor = endColor;
        pei.StreakFlag = streakFlg;
        pei.TextureID = ptcTex;

        apc.CreateEmitter(&pei);

        COMP_TYPE type = COMP_TYPE::A_PARTICLE;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, apc);
    }
    else if (compType == "animate")
    {
        AAnimateComponent aac(compName, nullptr);

        std::string initAni = getJsonNode(_json, _jsonPath + "/aac-init-animation")->
            GetString();
        float spdFactor = 1.f;
        if (!getJsonNode(_json, _jsonPath + "/aac-speed-factor")->IsNull())
        {
            spdFactor = getJsonNode(_json, _jsonPath + "/aac-speed-factor")->GetFloat();
        }
        aac.ChangeAnimationTo(initAni);
        aac.SetSpeedFactor(spdFactor);

        COMP_TYPE type = COMP_TYPE::A_ANIMATE;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aac);
    }
    else if (compType == "sprite")
    {
        ASpriteComponent asc(compName, nullptr);

        JsonNode texNameNode = getJsonNode(_json, _jsonPath + "/asc-texture");
        JsonNode billFlgNode = getJsonNode(_json, _jsonPath + "/asc-billboard");
        JsonNode sizeXNode = getJsonNode(_json, _jsonPath + "/asc-size/0");
        JsonNode sizeYNode = getJsonNode(_json, _jsonPath + "/asc-size/1");
        JsonNode texCUNode = getJsonNode(_json, _jsonPath + "/asc-tex-coord/0");
        JsonNode texCVNode = getJsonNode(_json, _jsonPath + "/asc-tex-coord/1");
        JsonNode texCULenNode = getJsonNode(_json, _jsonPath + "/asc-tex-coord/2");
        JsonNode texCVLenNode = getJsonNode(_json, _jsonPath + "/asc-tex-coord/3");

        assert(texNameNode && billFlgNode && sizeXNode && sizeYNode &&
            texCUNode && texCVNode && texCULenNode && texCVLenNode);

        asc.SetSpriteProperty(
            { sizeXNode->GetFloat(),sizeYNode->GetFloat() },
            { texCUNode->GetFloat(),texCVNode->GetFloat(),
            texCULenNode->GetFloat(),texCVLenNode->GetFloat() },
            billFlgNode->GetBool());

        JsonNode aniFlgNode = getJsonNode(_json, _jsonPath + "/asc-with-animation");
        if (aniFlgNode && !aniFlgNode->IsNull() && aniFlgNode->GetBool())
        {
            JsonNode strideUNode = getJsonNode(_json, _jsonPath + "/asc-stride/0");
            JsonNode strideVNode = getJsonNode(_json, _jsonPath + "/asc-stride/1");
            JsonNode maxCutNode = getJsonNode(_json, _jsonPath + "/asc-max-cut");
            JsonNode switchTimeNode = getJsonNode(_json, _jsonPath + "/asc-switch-time");
            JsonNode repeatFlgNode = getJsonNode(_json, _jsonPath + "/asc-repeat-flag");

            assert(strideUNode && strideVNode && maxCutNode &&
                switchTimeNode && repeatFlgNode);

            asc.SetAnimationProperty(
                { strideUNode->GetFloat(),strideVNode->GetFloat() },
                maxCutNode->GetUint(),
                repeatFlgNode->GetBool(),
                switchTimeNode->GetFloat());
        }

        asc.CreateGeoPointWithTexture(_node, texNameNode->GetString());

        COMP_TYPE type = COMP_TYPE::A_SPRITE;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, asc);
    }
    else
    {
        P_LOG(LOG_ERROR, "invlaid comp type : %s\n", compType.c_str());
        return;
    }
}

void ObjectFactory::CreateUiComp(SceneNode* _node, UiObject* _ui,
    JsonFile& _json, std::string _jsonPath)
{
    std::string compType = getJsonNode(_json, _jsonPath + "/type")->GetString();
    std::string compName = _ui->GetObjectName() + "-" + compType;

    if (compType == "transform")
    {
        UTransformComponent utc(compName, nullptr);

        float pos[3] = { 0.f };
        float ang[3] = { 0.f };
        float sca[3] = { 0.f };
        for (UINT i = 0; i < ARRAYSIZE(pos); i++)
        {
            pos[i] = getJsonNode(_json,
                _jsonPath + "/utc-init-position/" + std::to_string(i))->GetFloat();
            ang[i] = getJsonNode(_json,
                _jsonPath + "/utc-init-angle/" + std::to_string(i))->GetFloat();
            sca[i] = getJsonNode(_json,
                _jsonPath + "/utc-init-scale/" + std::to_string(i))->GetFloat();
        }
        utc.ForcePosition({ pos[0],pos[1],pos[2] });
        utc.ForceRotation({ ang[0],ang[1],ang[2] });
        utc.ForceScaling({ sca[0],sca[1],sca[2] });

        COMP_TYPE type = COMP_TYPE::U_TRANSFORM;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, utc);
    }
    else if (compType == "input")
    {
        UInputComponent uic(compName, nullptr);

        std::string inputFuncName = getJsonNode(_json,
            _jsonPath + "/uic-func-name")->GetString();
        auto found = mUiInputFuncPtrMap.find(inputFuncName);
        if (found == mUiInputFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid input func name : %s\n",
                inputFuncName.c_str());
            return;
        }
        uic.SetInputFunction(found->second);

        COMP_TYPE type = COMP_TYPE::U_INPUT;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uic);
    }
    else if (compType == "interact")
    {
        UInteractComponent uitc(compName, nullptr);

        std::string initFuncName = getJsonNode(_json,
            _jsonPath + "/uitc-init-func-name")->GetString();
        std::string updateFuncName = getJsonNode(_json,
            _jsonPath + "/uitc-update-func-name")->GetString();
        std::string destoryFuncName = getJsonNode(_json,
            _jsonPath + "/uitc-destory-func-name")->GetString();
        auto foundInit = mUiInteractInitFuncPtrMap.find(initFuncName);
        if (foundInit == mUiInteractInitFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid init func name : %s\n",
                initFuncName.c_str());
            return;
        }
        uitc.SetInitFunction(foundInit->second);
        auto foundUpdate = mUiInteractUpdateFuncPtrMap.find(updateFuncName);
        if (foundUpdate == mUiInteractUpdateFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid update func name : %s\n",
                updateFuncName.c_str());
            return;
        }
        uitc.SetUpdateFunction(foundUpdate->second);
        auto foundDestory = mUiInteractDestoryFuncPtrMap.find(destoryFuncName);
        if (foundDestory == mUiInteractDestoryFuncPtrMap.end())
        {
            P_LOG(LOG_ERROR, "invlaid destory func name : %s\n",
                destoryFuncName.c_str());
            return;
        }
        uitc.SetDestoryFunction(foundDestory->second);

        COMP_TYPE type = COMP_TYPE::U_INTERACT;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uitc);
    }
    else if (compType == "timer")
    {
        UTimerComponent utmc(compName, nullptr);

        UINT timerSize = getJsonNode(_json, _jsonPath + "/utmc-timers")->Size();
        for (UINT i = 0; i < timerSize; i++)
        {
            std::string timerName = getJsonNode(_json,
                _jsonPath + "/utmc-timers/" + std::to_string(i))->GetString();
            utmc.AddTimer(timerName);
        }

        COMP_TYPE type = COMP_TYPE::U_TIMER;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, utmc);
    }
    else if (compType == "audio")
    {
        UAudioComponent uauc(compName, nullptr);

        UINT audioSize = getJsonNode(_json, _jsonPath + "/uauc-sounds")->Size();
        for (UINT i = 0; i < audioSize; i++)
        {
            std::string soundName = getJsonNode(_json,
                _jsonPath + "/uauc-sounds/" + std::to_string(i))->GetString();
            uauc.AddAudio(soundName, *_node);
        }

        COMP_TYPE type = COMP_TYPE::U_AUDIO;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uauc);
    }
    else if (compType == "sprite")
    {
        USpriteComponent usc(compName, nullptr);

        DirectX::XMFLOAT4 offsetColor =
        {
            getJsonNode(_json, _jsonPath + "/usc-offset-color/0")->GetFloat(),
            getJsonNode(_json, _jsonPath + "/usc-offset-color/1")->GetFloat(),
            getJsonNode(_json, _jsonPath + "/usc-offset-color/2")->GetFloat(),
            getJsonNode(_json, _jsonPath + "/usc-offset-color/3")->GetFloat()
        };
        std::string texFile = getJsonNode(_json,
            _jsonPath + "/usc-tex-file")->GetString();

        usc.CreateSpriteMesh(_node, offsetColor, texFile);

        COMP_TYPE type = COMP_TYPE::U_SPRITE;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, usc);
    }
    else if (compType == "animate")
    {
        UAnimateComponent uac(compName, nullptr);

        UINT aniSize = getJsonNode(_json, _jsonPath + "/uac-animates")->Size();
        std::string aniArrayPath = _jsonPath + "/uac-animates/";
        for (UINT i = 0; i < aniSize; i++)
        {
            aniArrayPath = _jsonPath + "/uac-animates/" + std::to_string(i);

            std::string aniName = getJsonNode(_json,
                aniArrayPath + "/name")->GetString();
            std::string aniFile = getJsonNode(_json,
                aniArrayPath + "/tex-file")->GetString();
            DirectX::XMFLOAT2 stride =
            {
                getJsonNode(_json, aniArrayPath + "/u-v-stride/0")->GetFloat(),
                getJsonNode(_json, aniArrayPath + "/u-v-stride/1")->GetFloat()
            };
            UINT maxCount = getJsonNode(_json,
                aniArrayPath + "/max-count")->GetUint();
            float switchTime = getJsonNode(_json,
                aniArrayPath + "/switch-time")->GetFloat();
            bool repeatFlg = getJsonNode(_json,
                aniArrayPath + "/repeat-flag")->GetBool();

            uac.LoadAnimate(aniName, aniFile, stride, maxCount,
                repeatFlg, switchTime);
        }

        JsonNode initNode = getJsonNode(_json,
            _jsonPath + "/uac-init-ani");
        if (initNode && !initNode->IsNull())
        {
            uac.ChangeAnimateTo(initNode->GetString());
        }

        COMP_TYPE type = COMP_TYPE::U_ANIMATE;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uac);
    }
    else if (compType == "button")
    {
        UButtonComponent ubc(compName, nullptr);

        bool selected = getJsonNode(_json,
            _jsonPath + "/ubc-init-selected")->GetBool();
        ubc.SetIsBeingSelected(selected);
        JsonNode surdBtn = getJsonNode(_json, _jsonPath + "/ubc-up-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetUpBtnObjName(surdBtn->GetString());
        }
        surdBtn = getJsonNode(_json, _jsonPath + "/ubc-down-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetDownBtnObjName(surdBtn->GetString());
        }
        surdBtn = getJsonNode(_json, _jsonPath + "/ubc-left-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetLeftBtnObjName(surdBtn->GetString());
        }
        surdBtn = getJsonNode(_json, _jsonPath + "/ubc-right-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetRightBtnObjName(surdBtn->GetString());
        }

        COMP_TYPE type = COMP_TYPE::U_BUTTON;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, ubc);
    }
    else
    {
        P_LOG(LOG_ERROR, "invlaid comp type : %s\n", compType.c_str());
        return;
    }
}
