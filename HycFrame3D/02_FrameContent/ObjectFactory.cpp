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
    std::string jsonPath = "";
    if (modelRoot && modelRoot->Size())
    {
        UINT modelSize = modelRoot->Size();
        std::string meshName = "";
        std::string staticMatName = "copper";
        bool forceMat = false;
        RS_MATERIAL_INFO matInfo = {};
        std::string forceDiffuse = "";
        std::string forceNormal = "";
        std::string loadMode = "";
        RS_SUBMESH_DATA meshData = {};
        static SUBMESH_BONES bonesData = {};
        bonesData.clear();
        MESH_ANIMATION_DATA* animationData = nullptr;
        for (UINT i = 0; i < modelSize; i++)
        {
            meshName = "";
            int subIndex = 0;
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
                //matInfo.mDiffuseAlbedo = albedo;
                //matInfo.mFresnelR0 = fresnel;
                //matInfo.mShininess = shininess;
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
                std::string fileName = GetJsonNode(_json,
                    jsonPath + "/load-info/model-file")->GetString();
                std::string fileType = GetJsonNode(_json,
                    jsonPath + "/load-info/file-type")->GetString();
                auto subIndexNode = GetJsonNode(_json,
                    jsonPath + "/load-info/sub-mesh-index");
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
                LoadModelFile(fileName, type, subIndex, &meshData,
                    &bonesData, &animationData);
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
                float radius = GetJsonNode(_json,
                    jsonPath + "/load-info/radius")->GetFloat();
                UINT slice = GetJsonNode(_json,
                    jsonPath + "/load-info/slice-stack-count/0")->GetUint();
                UINT stack = GetJsonNode(_json,
                    jsonPath + "/load-info/slice-stack-count/1")->GetUint();
                meshData = GetRSRoot_DX11_Singleton()->MeshHelper()->
                    GeoGenerate()->CreateSphere(radius, slice, stack,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        GetJsonNode(_json,
                            jsonPath + "/load-info/tex-file")->GetString());
            }
            else if (loadMode == "program-geo-sphere")
            {
                float radius = GetJsonNode(_json,
                    jsonPath + "/load-info/radius")->GetFloat();
                UINT divide = GetJsonNode(_json,
                    jsonPath + "/load-info/divide")->GetUint();
                meshData = GetRSRoot_DX11_Singleton()->MeshHelper()->
                    GeoGenerate()->CreateGeometrySphere(radius, divide,
                        LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                        GetJsonNode(_json,
                            jsonPath + "/load-info/tex-file")->GetString());
            }
            else if (loadMode == "program-cylinder")
            {
                float topRadius = GetJsonNode(_json,
                    jsonPath + "/load-info/top-btm-het-size/0")->GetFloat();
                float bottomRadius = GetJsonNode(_json,
                    jsonPath + "/load-info/top-btm-het-size/1")->GetFloat();
                float height = GetJsonNode(_json,
                    jsonPath + "/load-info/top-btm-het-size/2")->GetFloat();
                UINT slice = GetJsonNode(_json,
                    jsonPath + "/load-info/slice-stack-count/0")->GetUint();
                UINT stack = GetJsonNode(_json,
                    jsonPath + "/load-info/slice-stack-count/1")->GetUint();
                meshData = GetRSRoot_DX11_Singleton()->MeshHelper()->
                    GeoGenerate()->CreateCylinder(bottomRadius, topRadius,
                        height, slice, stack, LAYOUT_TYPE::NORMAL_TANGENT_TEX,
                        false, {},
                        GetJsonNode(_json,
                            jsonPath + "/load-info/tex-file")->GetString());
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

            if (!meshData.mTextures.size())
            {
                P_LOG(LOG_ERROR, "invlaid model without diffuse : %s\n",
                    meshName.c_str());
                GetRSRoot_DX11_Singleton()->MeshHelper()->
                    ReleaseSubMesh(meshData);
                return;
            }

            _node->GetAssetsPool()->
                InsertNewIndexedMesh(meshName, meshData,
                    MESH_TYPE::OPACITY, subIndex,
                    &bonesData, animationData);
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
            _node->GetAssetsPool()->InsertNewSound(name);
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
    JsonNode uiRoot = GetJsonNode(_json, _jsonPath);
    if (!uiRoot)
    {
        P_LOG(LOG_ERROR, "failed to get ui root node : %s\n",
            _jsonPath.c_str());
        return;
    }

    std::string uiName = "";
    {
        JsonNode uiNameNode = GetJsonNode(_json, _jsonPath + "/ui-name");
        if (uiNameNode && uiNameNode->IsString())
        {
            uiName = uiNameNode->GetString();
        }
        else
        {
            P_LOG(LOG_ERROR, "invalid ui name in : %s\n", _jsonPath);
            return;
        }
    }

    UiObject ui(uiName, *_node);

    JsonNode compsRoot = GetJsonNode(_json, _jsonPath + "/components");
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
    else if (compType == "light")
    {
        ALightComponent alc(compName, nullptr);

        std::string lightTypeStr = GetJsonNode(_json,
            _jsonPath + "/light-type")->GetString();
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

        bool bloomFlag = GetJsonNode(_json,
            _jsonPath + "/with-bloom")->GetBool();
        bool shadowFlag = false;
        if (lightType == LIGHT_TYPE::DIRECT)
        {
            shadowFlag = GetJsonNode(_json,
                _jsonPath + "/with-shadow")->GetBool();
        }

        LIGHT_INFO li = {};
        CAM_INFO ci = {};
        li.mType = lightType;
        li.mPosition = { 0.f,0.f,0.f };
        li.mWithShadow = shadowFlag;
        li.mDirection.x = GetJsonNode(_json,
            _jsonPath + "/direction/0")->GetFloat();
        li.mDirection.y = GetJsonNode(_json,
            _jsonPath + "/direction/1")->GetFloat();
        li.mDirection.z = GetJsonNode(_json,
            _jsonPath + "/direction/2")->GetFloat();
        li.mTempIntensity = GetJsonNode(_json,
            _jsonPath + "/intensity")->GetFloat();
        li.mAlbedo.x = GetJsonNode(_json,
            _jsonPath + "/albedo/0")->GetFloat();
        li.mAlbedo.y = GetJsonNode(_json,
            _jsonPath + "/albedo/1")->GetFloat();
        li.mAlbedo.z = GetJsonNode(_json,
            _jsonPath + "/albedo/2")->GetFloat();
        li.mFalloffStart = GetJsonNode(_json,
            _jsonPath + "/fall-off-start-end/0")->GetFloat();
        li.mFalloffEnd = GetJsonNode(_json,
            _jsonPath + "/fall-off-start-end/1")->GetFloat();
        li.mSpotPower = GetJsonNode(_json,
            _jsonPath + "/spot-power")->GetFloat();
        if (GetJsonNode(_json, _jsonPath + "/cam-up-vec"))
        {
            ci.mType = LENS_TYPE::ORTHOGRAPHIC;
            ci.mPosition = li.mPosition;
            ci.mLookAt = li.mDirection;
            ci.mNearFarZ = { 0.f,1000.f };
            ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
            ci.mOWidthAndHeight = { 128.f * 9.5f,72.f * 9.5f };
            ci.mUpVec.x = GetJsonNode(_json,
                _jsonPath + "/cam-up-vec/0")->GetFloat();
            ci.mUpVec.y = GetJsonNode(_json,
                _jsonPath + "/cam-up-vec/1")->GetFloat();
            ci.mUpVec.z = GetJsonNode(_json,
                _jsonPath + "/cam-up-vec/2")->GetFloat();
        }

        alc.AddLight(li, bloomFlag, shadowFlag, ci);

        COMP_TYPE type = COMP_TYPE::A_LIGHT;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, alc);
    }
    else if (compType == "audio")
    {
        AAudioComponent aac(compName, nullptr);

        UINT audioSize = GetJsonNode(_json, _jsonPath + "/sounds")->Size();
        for (UINT i = 0; i < audioSize; i++)
        {
            std::string soundName = GetJsonNode(_json,
                _jsonPath + "/sounds/" + std::to_string(i))->GetString();
            aac.AddAudio(soundName, *_node);
        }

        COMP_TYPE type = COMP_TYPE::A_AUDIO;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aac);
    }
    else if (compType == "particle")
    {
        AParticleComponent apc(compName, nullptr);

        std::string ptcTexName = GetJsonNode(_json,
            _jsonPath + "/texture-name")->GetString();
        PARTICLE_TEXTURE ptcTex = PARTICLE_TEXTURE::PARTICLE_TEXTURE_SIZE;
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

        float emitPreSec = GetJsonNode(_json,
            _jsonPath + "/emit-per-second")->GetFloat();
        DirectX::XMFLOAT3 velocity =
        {
            GetJsonNode(_json,
            _jsonPath + "/velocity/0")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/velocity/1")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/velocity/2")->GetFloat()
        };
        DirectX::XMFLOAT3 posVariance =
        {
            GetJsonNode(_json,
            _jsonPath + "/pos-variance/0")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/pos-variance/1")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/pos-variance/2")->GetFloat()
        };
        float velVariance = GetJsonNode(_json,
            _jsonPath + "/vel-variance")->GetFloat();
        DirectX::XMFLOAT3 acceleration =
        {
            GetJsonNode(_json,
            _jsonPath + "/acceleration/0")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/acceleration/1")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/acceleration/2")->GetFloat()
        };
        float mass = GetJsonNode(_json,
            _jsonPath + "/particle-mass")->GetFloat();
        float lifeSpan = GetJsonNode(_json,
            _jsonPath + "/life-span")->GetFloat();
        float startSize = GetJsonNode(_json,
            _jsonPath + "/start-end-size/0")->GetFloat();
        float endSize = GetJsonNode(_json,
            _jsonPath + "/start-end-size/1")->GetFloat();
        DirectX::XMFLOAT4 startColor =
        {
            GetJsonNode(_json,
            _jsonPath + "/start-color/0")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/start-color/1")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/start-color/2")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/start-color/3")->GetFloat()
        };
        DirectX::XMFLOAT4 endColor =
        {
            GetJsonNode(_json,
            _jsonPath + "/end-color/0")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/end-color/1")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/end-color/2")->GetFloat(),
            GetJsonNode(_json,
            _jsonPath + "/end-color/3")->GetFloat()
        };
        bool streakFlg = GetJsonNode(_json,
            _jsonPath + "/streak-flag")->GetBool();

        PARTICLE_EMITTER_INFO pei = {};
        pei.mEmitNumPerSecond = emitPreSec;
        pei.mVelocity = velocity;
        pei.mPosVariance = posVariance;
        pei.mVelVariance = velVariance;
        pei.mAcceleration = acceleration;
        pei.mParticleMass = mass;
        pei.mLifeSpan = lifeSpan;
        pei.mOffsetStartSize = startSize;
        pei.mOffsetEndSize = endSize;
        pei.mOffsetStartColor = startColor;
        pei.mOffsetEndColor = endColor;
        pei.mEnableStreak = streakFlg;
        pei.mTextureID = ptcTex;

        apc.CreateEmitter(&pei);

        COMP_TYPE type = COMP_TYPE::A_PARTICLE;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, apc);
    }
    else if (compType == "animate")
    {
        AAnimateComponent aanc(compName, nullptr);

        std::string initAni = GetJsonNode(_json, _jsonPath + "/init-animation")->
            GetString();
        float spdFactor = 1.f;
        if (!GetJsonNode(_json, _jsonPath + "/speed-factor")->IsNull())
        {
            spdFactor = GetJsonNode(_json, _jsonPath + "/speed-factor")->GetFloat();
        }
        aanc.ChangeAnimationTo(initAni);
        aanc.SetSpeedFactor(spdFactor);

        COMP_TYPE type = COMP_TYPE::A_ANIMATE;
        _actor->AddAComponent(type);
        _node->GetComponentContainer()->AddComponent(type, aanc);
    }
    else
    {
        P_LOG(LOG_ERROR, "invlaid comp type : %s\n", compType.c_str());
        return;
    }
}

void ObjectFactory::CreateUiComp(SceneNode* _node, UiObject* _ui,
    JsonFile* _json, std::string _jsonPath)
{
    JsonNode compRoot = GetJsonNode(_json, _jsonPath);
    std::string compType = GetJsonNode(_json, _jsonPath + "/type")->GetString();
    std::string compName = _ui->GetObjectName() + "-" + compType;

    if (compType == "transform")
    {
        UTransformComponent utc(compName, nullptr);

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

        std::string inputFuncName = GetJsonNode(_json,
            _jsonPath + "/func-name")->GetString();
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

        std::string initFuncName = GetJsonNode(_json,
            _jsonPath + "/init-func-name")->GetString();
        std::string updateFuncName = GetJsonNode(_json,
            _jsonPath + "/update-func-name")->GetString();
        std::string destoryFuncName = GetJsonNode(_json,
            _jsonPath + "/destory-func-name")->GetString();
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

        UINT timerSize = GetJsonNode(_json, _jsonPath + "/timers")->Size();
        for (UINT i = 0; i < timerSize; i++)
        {
            std::string timerName = GetJsonNode(_json,
                _jsonPath + "/timers/" + std::to_string(i))->GetString();
            utmc.AddTimer(timerName);
        }

        COMP_TYPE type = COMP_TYPE::U_TIMER;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, utmc);
    }
    else if (compType == "audio")
    {
        UAudioComponent uac(compName, nullptr);

        UINT audioSize = GetJsonNode(_json, _jsonPath + "/sounds")->Size();
        for (UINT i = 0; i < audioSize; i++)
        {
            std::string soundName = GetJsonNode(_json,
                _jsonPath + "/sounds/" + std::to_string(i))->GetString();
            uac.AddAudio(soundName, *_node);
        }

        COMP_TYPE type = COMP_TYPE::U_AUDIO;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uac);
    }
    else if (compType == "sprite")
    {
        USpriteComponent usc(compName, nullptr);

        DirectX::XMFLOAT4 offsetColor =
        {
            GetJsonNode(_json, _jsonPath + "/offset-color/0")->GetFloat(),
            GetJsonNode(_json, _jsonPath + "/offset-color/1")->GetFloat(),
            GetJsonNode(_json, _jsonPath + "/offset-color/2")->GetFloat(),
            GetJsonNode(_json, _jsonPath + "/offset-color/3")->GetFloat()
        };
        std::string texFile = GetJsonNode(_json,
            _jsonPath + "/tex-file")->GetString();

        usc.CreateSpriteMesh(_node, offsetColor, texFile);

        COMP_TYPE type = COMP_TYPE::U_SPRITE;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, usc);
    }
    else if (compType == "animate")
    {
        UAnimateComponent uamc(compName, nullptr);

        UINT aniSize = GetJsonNode(_json, _jsonPath + "/animates")->Size();
        std::string aniArrayPath = _jsonPath + "/animates/";
        for (UINT i = 0; i < aniSize; i++)
        {
            aniArrayPath = _jsonPath + "/animates/" + std::to_string(i);

            std::string aniName = GetJsonNode(_json,
                aniArrayPath + "/name")->GetString();
            std::string aniFile = GetJsonNode(_json,
                aniArrayPath + "/tex-file")->GetString();
            DirectX::XMFLOAT2 stride =
            {
                GetJsonNode(_json, aniArrayPath + "/u-v-stride/0")->GetFloat(),
                GetJsonNode(_json, aniArrayPath + "/u-v-stride/1")->GetFloat()
            };
            UINT maxCount = GetJsonNode(_json,
                aniArrayPath + "/max-count")->GetUint();
            float switchTime = GetJsonNode(_json,
                aniArrayPath + "/switch-time")->GetFloat();
            bool repeatFlg = GetJsonNode(_json,
                aniArrayPath + "/repeat-flag")->GetBool();

            uamc.LoadAnimate(aniName, aniFile, stride, maxCount,
                repeatFlg, switchTime);
        }

        JsonNode initNode = GetJsonNode(_json,
            _jsonPath + "/init-ani");
        if (initNode && !initNode->IsNull())
        {
            uamc.ChangeAnimateTo(initNode->GetString());
        }

        COMP_TYPE type = COMP_TYPE::U_ANIMATE;
        _ui->AddUComponent(type);
        _node->GetComponentContainer()->AddComponent(type, uamc);
    }
    else if (compType == "button")
    {
        UButtonComponent ubc(compName, nullptr);

        bool selected = GetJsonNode(_json,
            _jsonPath + "/init-selected")->GetBool();
        ubc.SetIsBeingSelected(selected);
        JsonNode surdBtn = GetJsonNode(_json, _jsonPath + "/up-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetUpBtnObjName(surdBtn->GetString());
        }
        surdBtn = GetJsonNode(_json, _jsonPath + "/down-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetDownBtnObjName(surdBtn->GetString());
        }
        surdBtn = GetJsonNode(_json, _jsonPath + "/left-btn");
        if (surdBtn && !surdBtn->IsNull())
        {
            ubc.SetLeftBtnObjName(surdBtn->GetString());
        }
        surdBtn = GetJsonNode(_json, _jsonPath + "/right-btn");
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
