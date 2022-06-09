#include "01_MaterialEditor.h"
#include "RSRoot_DX11.h"
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

static ATransformComponent* g_PointLightAtc = nullptr;
static ATransformComponent* g_MaterialBallAtc = nullptr;
static RS_MATERIAL_INFO* g_Material = nullptr;

void OutputThisMaterialInfo()
{
    std::string matInfo = "Material Info\n";
    matInfo += "===============================\n";
    matInfo += "FresnelR0 :\t\t" + std::to_string(g_Material->mFresnelR0.x);
    matInfo += ", " + std::to_string(g_Material->mFresnelR0.y);
    matInfo += ", " + std::to_string(g_Material->mFresnelR0.z);
    matInfo += "\n";
    matInfo += "SubSurface :\t" + std::to_string(g_Material->mSubSurface) + ", \t";
    matInfo += "Metallic :\t\t\t" + std::to_string(g_Material->mMetallic) + "\n";
    matInfo += "Specular :\t\t" + std::to_string(g_Material->mSpecular) + ", \t";
    matInfo += "SpecularTint :\t\t" + std::to_string(g_Material->mSpecularTint) + "\n";
    matInfo += "Roughness :\t\t" + std::to_string(g_Material->mRoughness) + ", \t";
    matInfo += "Anisotropic :\t\t" + std::to_string(g_Material->mAnisotropic) + "\n";
    matInfo += "Sheen :\t\t\t" + std::to_string(g_Material->mSheen) + ", \t";
    matInfo += "SheenTint :\t\t\t" + std::to_string(g_Material->mSheenTint) + "\n";
    matInfo += "Clearcoat :\t\t" + std::to_string(g_Material->mClearcoat) + ", \t";
    matInfo += "ClearcoatGloss :\t" + std::to_string(g_Material->mClearcoatGloss) + "\n";
    matInfo += "===============================\n";
    P_LOG(LOG_DEBUG, "%s", matInfo.c_str());
}

void MatEditorInput(AInputComponent* _aic, Timer& _timer)
{
    if (InputInterface::IsKeyPushedInSingle(KB_F5))
    {
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("material-scene", "material-scene.json");
    }

    if (InputInterface::IsKeyPushedInSingle(KB_P))
    {
        static bool simp = false;
        std::string basic = "light-pipeline";
        std::string simple = "simple-pipeline";
        if (simp)
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->
                SetPipeline(basic);
        }
        else
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->
                SetPipeline(simple);
        }
        simp = !simp;
    }

    const float deltatime = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(M_LEFTBTN))
    {
        auto mouseOffset = InputInterface::GetMouseOffset();
        float horiR = -mouseOffset.x * deltatime / 500.f;
        float vertR = -mouseOffset.y * deltatime / 500.f;
        g_MaterialBallAtc->Rotate({ vertR,horiR,0.f });
    }

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        g_PointLightAtc->TranslateZAsix(0.1f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        g_PointLightAtc->TranslateZAsix(-0.1f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        g_PointLightAtc->TranslateXAsix(0.1f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        g_PointLightAtc->TranslateXAsix(-0.1f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_E))
    {
        g_PointLightAtc->TranslateYAsix(0.1f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_Q))
    {
        g_PointLightAtc->TranslateYAsix(-0.1f * deltatime);
    }

    float& editValue = g_Material->mRoughness;
    if (InputInterface::IsKeyDownInSingle(KB_RIGHT))
    {
        editValue += deltatime / 1000.f;
        if (editValue > 1.f) { editValue = 1.f; }
    }
    if (InputInterface::IsKeyDownInSingle(KB_LEFT))
    {
        editValue -= deltatime / 1000.f;
        if (editValue < 0.f) { editValue = 0.f; }
    }

    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        OutputThisMaterialInfo();
    }
}

bool MatEditorInit(AInteractComponent* _aitc)
{
    std::string basic = "light-pipeline";
    GetRSRoot_DX11_Singleton()->PipelinesManager()->SetPipeline(basic);

    g_PointLightAtc = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("point-light-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_PointLightAtc) { return false; }

    g_MaterialBallAtc = _aitc->GetActorOwner()->
        GetSceneNode().GetActorObject("mat-ball-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_MaterialBallAtc) { return false; }

    auto& submeshName = (*_aitc->GetActorOwner()->
        GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted("mat-ball"))[0];
    g_Material = &(_aitc->GetActorOwner()->
        GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(submeshName)->
        mInstanceMap.begin()->second.mMaterialData);
    if (!g_Material) { return false; }

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
}
