#include "01_MaterialEditor.h"
#include "RSRoot_DX11.h"
#include "RSStaticResources.h"
#include "RSPipelinesManager.h"

void RegisterMaterialEditor(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(MatEditorInput),MatEditorInput });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(MatEditorInit),MatEditorInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(MatEditorUpdate),MatEditorUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(MatEditorDestory),MatEditorDestory });
}

enum class MAT_TYPE
{
    SUBSURFACE = KB_1,
    METALLIC = KB_2,
    SPECULAR = KB_3,
    SPECULARTINT = KB_4,
    ROUGHNESS = KB_5,
    ANISOTROPIC = KB_6,
    SHEEN = KB_7,
    SHEENTINT = KB_8,
    CLEARCOAT = KB_9,
    CLEARCOATGLOSS = KB_0
};

static ATransformComponent* g_PointLightAtc = nullptr;
static ATransformComponent* g_MaterialBallAtc = nullptr;
static RS_MATERIAL_DATA* g_Material = nullptr;
static MAT_TYPE g_EditingMatTerm = MAT_TYPE::ROUGHNESS;
static SUBMESH_DATA* g_MatBallMesh = nullptr;

void OutputThisMaterialInfo()
{
    std::string matInfo = "Material Info\n";
    matInfo += "===============================\n";
    matInfo += "FresnelR0 :\t\t" + std::to_string(g_Material->FresnelR0.x);
    matInfo += ", " + std::to_string(g_Material->FresnelR0.y);
    matInfo += ", " + std::to_string(g_Material->FresnelR0.z);
    matInfo += "\n";
    matInfo += "SubSurface :\t" + std::to_string(g_Material->SubSurface) + ", \t";
    matInfo += "Metallic :\t\t\t" + std::to_string(g_Material->Metallic) + "\n";
    matInfo += "Specular :\t\t" + std::to_string(g_Material->Specular) + ", \t";
    matInfo += "SpecularTint :\t\t" + std::to_string(g_Material->SpecularTint) + "\n";
    matInfo += "Roughness :\t\t" + std::to_string(g_Material->Roughness) + ", \t";
    matInfo += "Anisotropic :\t\t" + std::to_string(g_Material->Anisotropic) + "\n";
    matInfo += "Sheen :\t\t\t" + std::to_string(g_Material->Sheen) + ", \t";
    matInfo += "SheenTint :\t\t\t" + std::to_string(g_Material->SheenTint) + "\n";
    matInfo += "Clearcoat :\t\t" + std::to_string(g_Material->Clearcoat) + ", \t";
    matInfo += "ClearcoatGloss :\t" + std::to_string(g_Material->ClearcoatGloss) + "\n";
    matInfo += "===============================\n";
    P_LOG(LOG_DEBUG, "%s", matInfo.c_str());
}

void MatEditorInput(AInputComponent* _aic, Timer& _timer)
{
    if (input::isKeyPushedInSingle(KB_F5))
    {
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("material-scene", "material-scene.json");
    }

    if (input::isKeyPushedInSingle(KB_P))
    {
        static bool simp = false;
        std::string basic = "light-pipeline";
        std::string simple = "simple-pipeline";
        if (simp)
        {
            getRSDX11RootInstance()->getPipelinesManager()->
                setPipeline(basic);
        }
        else
        {
            getRSDX11RootInstance()->getPipelinesManager()->
                setPipeline(simple);
        }
        simp = !simp;
    }

    const float deltatime = _timer.floatDeltaTime();

    if (input::isKeyDownInSingle(M_LEFTBTN))
    {
        auto mouseOffset = input::getMouseOffset();
        float horiR = -mouseOffset.x * deltatime / 500.f;
        float vertR = -mouseOffset.y * deltatime / 500.f;
        g_MaterialBallAtc->rotate({ vertR,horiR,0.f });
    }

    if (input::isKeyDownInSingle(KB_W))
    {
        g_PointLightAtc->translateZAsix(0.1f * deltatime);
    }
    if (input::isKeyDownInSingle(KB_S))
    {
        g_PointLightAtc->translateZAsix(-0.1f * deltatime);
    }
    if (input::isKeyDownInSingle(KB_D))
    {
        g_PointLightAtc->translateXAsix(0.1f * deltatime);
    }
    if (input::isKeyDownInSingle(KB_A))
    {
        g_PointLightAtc->translateXAsix(-0.1f * deltatime);
    }
    if (input::isKeyDownInSingle(KB_E))
    {
        g_PointLightAtc->translateYAsix(0.1f * deltatime);
    }
    if (input::isKeyDownInSingle(KB_Q))
    {
        g_PointLightAtc->translateYAsix(-0.1f * deltatime);
    }

    UINT check = 0;
    switch (MAT_TYPE::SUBSURFACE)
    {
    case MAT_TYPE::SUBSURFACE:
        check = (UINT)MAT_TYPE::SUBSURFACE;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing subsurface\n");
            break;
        }
    case MAT_TYPE::METALLIC:
        check = (UINT)MAT_TYPE::METALLIC;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing metallic\n");
            break;
        }
    case MAT_TYPE::SPECULAR:
        check = (UINT)MAT_TYPE::SPECULAR;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing specular\n");
            break;
        }
    case MAT_TYPE::SPECULARTINT:
        check = (UINT)MAT_TYPE::SPECULARTINT;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing specular tint\n");
            break;
        }
    case MAT_TYPE::ROUGHNESS:
        check = (UINT)MAT_TYPE::ROUGHNESS;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing roughness\n");
            break;
        }
    case MAT_TYPE::ANISOTROPIC:
        check = (UINT)MAT_TYPE::ANISOTROPIC;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing anisotropic\n");
            break;
        }
    case MAT_TYPE::SHEEN:
        check = (UINT)MAT_TYPE::SHEEN;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing sheen\n");
            break;
        }
    case MAT_TYPE::SHEENTINT:
        check = (UINT)MAT_TYPE::SHEENTINT;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing sheen tint\n");
            break;
        }
    case MAT_TYPE::CLEARCOAT:
        check = (UINT)MAT_TYPE::CLEARCOAT;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing clearcoat\n");
            break;
        }
    case MAT_TYPE::CLEARCOATGLOSS:
        check = (UINT)MAT_TYPE::CLEARCOATGLOSS;
        if (input::isKeyPushedInSingle(check))
        {
            g_EditingMatTerm = (MAT_TYPE)check;
            P_LOG(LOG_DEBUG, "editing clearcoat gloss\n");
            break;
        }
    }

    float* editValue = nullptr;
    switch (g_EditingMatTerm)
    {
    case MAT_TYPE::SUBSURFACE:
        editValue = &g_Material->SubSurface;
        break;
    case MAT_TYPE::METALLIC:
        editValue = &g_Material->Metallic;
        break;
    case MAT_TYPE::SPECULAR:
        editValue = &g_Material->Specular;
        break;
    case MAT_TYPE::SPECULARTINT:
        editValue = &g_Material->SpecularTint;
        break;
    case MAT_TYPE::ROUGHNESS:
        editValue = &g_Material->Roughness;
        break;
    case MAT_TYPE::ANISOTROPIC:
        editValue = &g_Material->Anisotropic;
        break;
    case MAT_TYPE::SHEEN:
        editValue = &g_Material->Sheen;
        break;
    case MAT_TYPE::SHEENTINT:
        editValue = &g_Material->SheenTint;
        break;
    case MAT_TYPE::CLEARCOAT:
        editValue = &g_Material->Clearcoat;
        break;
    case MAT_TYPE::CLEARCOATGLOSS:
        editValue = &g_Material->ClearcoatGloss;
        break;
    }
    assert(editValue);
    if (input::isKeyDownInSingle(KB_RIGHT))
    {
        *editValue += deltatime / 1000.f;
        if (*editValue > 1.f) { *editValue = 1.f; }
        getRSDX11RootInstance()->getStaticResources()->remapMaterialData();
    }
    if (input::isKeyDownInSingle(KB_LEFT))
    {
        *editValue -= deltatime / 1000.f;
        if (*editValue < 0.f) { *editValue = 0.f; }
        getRSDX11RootInstance()->getStaticResources()->remapMaterialData();
    }

    float& factor = g_MatBallMesh->InstanceMap.begin()->
        second.MaterialData.InterpolateFactor;
    if (input::isKeyDownInSingle(KB_COMMA))
    {
        factor -= deltatime / 1000.f;
        if (factor < 0.f) { factor = 0.f; }
    }
    if (input::isKeyDownInSingle(KB_PERIOD))
    {
        factor += deltatime / 1000.f;
        if (factor > 1.f) { factor = 1.f; }
    }
    if (input::isKeyPushedInSingle(KB_SLASH))
    {
        P_LOG(LOG_DEBUG, "current material factor : %f\n", factor);
    }

    if (input::isKeyPushedInSingle(KB_RETURN))
    {
        OutputThisMaterialInfo();
    }
}

bool MatEditorInit(AInteractComponent* _aitc)
{
    std::string basic = "light-pipeline";
    getRSDX11RootInstance()->getPipelinesManager()->setPipeline(basic);

    g_PointLightAtc = _aitc->getActorObject("point-light-actor")->
        GetComponent<ATransformComponent>();
    if (!g_PointLightAtc) { return false; }

    g_MaterialBallAtc = _aitc->getActorObject("mat-ball-actor")->
        GetComponent<ATransformComponent>();
    if (!g_MaterialBallAtc) { return false; }

    g_MatBallMesh = &(*_aitc->getSceneNode().GetAssetsPool()->
        getSubMeshIfExisted("mat-ball0"));
    if (!g_MatBallMesh) { return false; }

    g_Material = getRSDX11RootInstance()->getStaticResources()->
        getMaterialDataPtrForTest();
    if (!g_Material) { return false; }

    g_EditingMatTerm = MAT_TYPE::ROUGHNESS;

    return true;
}

void MatEditorUpdate(AInteractComponent* _aitc, Timer& _timer)
{

}

void MatEditorDestory(AInteractComponent* _aitc)
{
    g_PointLightAtc = nullptr;
    g_MaterialBallAtc = nullptr;
    g_Material = nullptr;
    g_MatBallMesh = nullptr;
}
