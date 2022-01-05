#include "ButtonProcess.h"
#include "FadeProcess.h"

void RegisterButtonProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(NormalBtnInput),NormalBtnInput });
}

void NormalBtnInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP)) { ubc->SelectUpBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN)) { ubc->SelectDownBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT)) { ubc->SelectLeftBtn(); }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT)) { ubc->SelectRightBtn(); }

    if ((ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN)) ||
        (InputInterface::IsKeyPushedInSingle(KB_RETURN) && ubc->IsBeingSelected()))
    {
        if (ubc->GetCompName() == "back-title-btn-ui-button")
        {

        }
        else if (ubc->GetCompName() == "tutorial-btn-ui-button")
        {

        }
        else if (ubc->GetCompName() == "route1-btn-ui-button")
        {
            P_LOG(LOG_DEBUG, "to run\n");
            _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
                LoadSceneNode("run-scene", "run-scene.json");
        }
        else if (ubc->GetCompName() == "route2-btn-ui-button")
        {
            P_LOG(LOG_DEBUG, "to run2\n");
            _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
                LoadSceneNode("route2-scene", "route2-scene.json");
        }
        else if (ubc->GetCompName() == "quit-btn-ui-button")
        {
            PostQuitMessage(0);
        }
    }
}
