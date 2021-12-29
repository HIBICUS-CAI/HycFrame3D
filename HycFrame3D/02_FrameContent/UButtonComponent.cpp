#include "UButtonComponent.h"
#include "UiObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "UTransformComponent.h"
#include "WM_Interface.h"
#include "RSRoot_DX11.h"
#include "RSMeshHelper.h"

static UButtonComponent* g_SelectedBtnCompPtr = nullptr;

static bool g_CanThisFrameProcessSelect = true;

static MESH_DATA* g_SelectTexMeshPtr = nullptr;

UButtonComponent::UButtonComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mSurroundBtnObjectNames({ NULL_BTN,NULL_BTN,NULL_BTN,NULL_BTN }),
    mIsSelected(false), mUseMouseSelectFlg(false),
    mWndWidth(0.f), mWndHeight(0.f)
{

}

UButtonComponent::UButtonComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mSurroundBtnObjectNames({ NULL_BTN,NULL_BTN,NULL_BTN,NULL_BTN }),
    mIsSelected(false), mUseMouseSelectFlg(false),
    mWndWidth(0.f), mWndHeight(0.f)
{

}

UButtonComponent::~UButtonComponent()
{

}

bool UButtonComponent::Init()
{
    if (g_SelectedBtnCompPtr) { g_SelectedBtnCompPtr = nullptr; }
    if (g_SelectTexMeshPtr) { g_SelectTexMeshPtr = nullptr; }
    g_CanThisFrameProcessSelect = true;

    RECT wndRect = {};
    GetClientRect(WindowInterface::GetWindowPtr()->GetWndHandle(), &wndRect);
    mWndWidth = (float)(wndRect.right - wndRect.left);
    mWndHeight = (float)(wndRect.bottom - wndRect.top);

    if (mIsSelected) { g_SelectedBtnCompPtr = this; }

    MESH_DATA* mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted(SELECTED_BTN_SPRITE_NAME);
    if (!mesh)
    {
        RS_SUBMESH_DATA btnSelect = GetRSRoot_DX11_Singleton()->
            MeshHelper()->GeoGenerate()->CreateSpriteRect(
                LAYOUT_TYPE::NORMAL_TANGENT_TEX, SELECTED_BTN_TEX_NAME);
        GetUiOwner()->GetSceneNode().GetAssetsPool()->InsertNewMesh(
            SELECTED_BTN_SPRITE_NAME, btnSelect, MESH_TYPE::UI_SPRITE);
        mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(SELECTED_BTN_SPRITE_NAME);

        RS_INSTANCE_DATA id = {};
        id.mCustomizedData1 = { 1.f,1.f,1.f,1.f };
        id.mCustomizedData2 = { 0.f,0.f,1.f,1.f };
        mesh->mInstanceMap.insert({ SELECTED_BTN_SPRITE_NAME,id });

        g_SelectTexMeshPtr = mesh;
    }

    return true;
}

void UButtonComponent::Update(Timer& _timer)
{
    if (!g_CanThisFrameProcessSelect && !mUseMouseSelectFlg)
    {
        g_CanThisFrameProcessSelect = true;
    }

    if (mIsSelected)
    {
        g_SelectedBtnCompPtr = this;
        SyncDataFromTransform();
    }
}

void UButtonComponent::Destory()
{
    if (g_SelectedBtnCompPtr) { g_SelectedBtnCompPtr = nullptr; }
    if (g_SelectTexMeshPtr) { g_SelectTexMeshPtr = nullptr; }
    g_CanThisFrameProcessSelect = true;
}

void UButtonComponent::SetIsBeingSelected(bool _beingSelected)
{
    mIsSelected = _beingSelected;
}

bool UButtonComponent::IsBeingSelected() const
{
    return mIsSelected;
}

void UButtonComponent::SelectUpBtn()
{
    if (mIsSelected && g_CanThisFrameProcessSelect && !mUseMouseSelectFlg &&
        mSurroundBtnObjectNames[BTN_UP] != NULL_BTN)
    {
        auto upUi = GetUiOwner()->GetSceneNode().
            GetUiObject(mSurroundBtnObjectNames[BTN_UP]);
        if (!upUi) { return; }

        auto upUbc = upUi->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
        if (!upUbc) { return; }

        SetIsBeingSelected(false);
        upUbc->SetIsBeingSelected(true);
        g_CanThisFrameProcessSelect = false;
    }
}

void UButtonComponent::SelectDownBtn()
{
    if (mIsSelected && g_CanThisFrameProcessSelect && !mUseMouseSelectFlg &&
        mSurroundBtnObjectNames[BTN_DOWN] != NULL_BTN)
    {
        auto downUi = GetUiOwner()->GetSceneNode().
            GetUiObject(mSurroundBtnObjectNames[BTN_DOWN]);
        if (!downUi) { return; }

        auto downUbc = downUi->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
        if (!downUbc) { return; }

        SetIsBeingSelected(false);
        downUbc->SetIsBeingSelected(true);
        g_CanThisFrameProcessSelect = false;
    }
}

void UButtonComponent::SelectLeftBtn()
{
    if (mIsSelected && g_CanThisFrameProcessSelect && !mUseMouseSelectFlg &&
        mSurroundBtnObjectNames[BTN_LEFT] != NULL_BTN)
    {
        auto leftUi = GetUiOwner()->GetSceneNode().
            GetUiObject(mSurroundBtnObjectNames[BTN_LEFT]);
        if (!leftUi) { return; }

        auto leftUbc = leftUi->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
        if (!leftUbc) { return; }

        SetIsBeingSelected(false);
        leftUbc->SetIsBeingSelected(true);
        g_CanThisFrameProcessSelect = false;
    }
}

void UButtonComponent::SelectRightBtn()
{
    if (mIsSelected && g_CanThisFrameProcessSelect && !mUseMouseSelectFlg &&
        mSurroundBtnObjectNames[BTN_RIGHT] != NULL_BTN)
    {
        auto rightUi = GetUiOwner()->GetSceneNode().
            GetUiObject(mSurroundBtnObjectNames[BTN_RIGHT]);
        if (!rightUi) { return; }

        auto rightUbc = rightUi->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
        if (!rightUbc) { return; }

        SetIsBeingSelected(false);
        rightUbc->SetIsBeingSelected(true);
        g_CanThisFrameProcessSelect = false;
    }
}

UButtonComponent* UButtonComponent::GetUpBtn()
{
    auto ui = GetUiOwner()->GetSceneNode().
        GetUiObject(mSurroundBtnObjectNames[BTN_UP]);
    if (!ui)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
            mSurroundBtnObjectNames[BTN_UP].c_str());
        return nullptr;
    }

    auto ubc = ui->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
            mSurroundBtnObjectNames[BTN_UP].c_str());
        return nullptr;
    }
    else
    {
        return ubc;
    }
}

UButtonComponent* UButtonComponent::GetDownBtn()
{
    auto ui = GetUiOwner()->GetSceneNode().
        GetUiObject(mSurroundBtnObjectNames[BTN_DOWN]);
    if (!ui)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
            mSurroundBtnObjectNames[BTN_DOWN].c_str());
        return nullptr;
    }

    auto ubc = ui->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
            mSurroundBtnObjectNames[BTN_DOWN].c_str());
        return nullptr;
    }
    else
    {
        return ubc;
    }
}

UButtonComponent* UButtonComponent::GetLeftBtn()
{
    auto ui = GetUiOwner()->GetSceneNode().
        GetUiObject(mSurroundBtnObjectNames[BTN_LEFT]);
    if (!ui)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
            mSurroundBtnObjectNames[BTN_LEFT].c_str());
        return nullptr;
    }

    auto ubc = ui->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
            mSurroundBtnObjectNames[BTN_LEFT].c_str());
        return nullptr;
    }
    else
    {
        return ubc;
    }
}

UButtonComponent* UButtonComponent::GetRightBtn()
{
    auto ui = GetUiOwner()->GetSceneNode().
        GetUiObject(mSurroundBtnObjectNames[BTN_RIGHT]);
    if (!ui)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
            mSurroundBtnObjectNames[BTN_RIGHT].c_str());
        return nullptr;
    }

    auto ubc = ui->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc)
    {
        P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
            mSurroundBtnObjectNames[BTN_RIGHT].c_str());
        return nullptr;
    }
    else
    {
        return ubc;
    }
}

void UButtonComponent::SetUpBtnObjName(std::string _upBtn)
{
    mSurroundBtnObjectNames[BTN_UP] = _upBtn;
}

void UButtonComponent::SetDownBtnObjName(std::string _downBtn)
{
    mSurroundBtnObjectNames[BTN_DOWN] = _downBtn;
}

void UButtonComponent::SetLeftBtnObjName(std::string _leftBtn)
{
    mSurroundBtnObjectNames[BTN_LEFT] = _leftBtn;
}

void UButtonComponent::SetRightBtnObjName(std::string _rightBtn)
{
    mSurroundBtnObjectNames[BTN_RIGHT] = _rightBtn;
}

void UButtonComponent::SyncDataFromTransform()
{
    UTransformComponent* utc = GetUiOwner()->
        GetUComponent<UTransformComponent>(COMP_TYPE::U_TRANSFORM);
#ifdef _DEBUG
    assert(utc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = utc->GetProcessingPosition();
    DirectX::XMFLOAT3 angle = utc->GetProcessingRotation();
    DirectX::XMFLOAT3 scale = utc->GetProcessingScaling();

    auto& map = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted(SELECTED_BTN_SPRITE_NAME)->mInstanceMap;

    for (auto& ins : map)
    {
        auto& ins_data = ins.second;

        DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z),
            DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z));
        mat = DirectX::XMMatrixMultiply(mat,
            DirectX::XMMatrixTranslation(world.x, world.y, world.z));
        DirectX::XMStoreFloat4x4(&(ins_data.mWorldMat), mat);

        break;
    }
}
