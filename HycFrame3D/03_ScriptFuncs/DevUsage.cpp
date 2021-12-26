#include "DevUsage.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "UiObject.h"
#include "ID_Interface.h"
#include "ComponentContainer.h"
#include "ATransformComponent.h"
#include "UAnimateComponent.h"
#include "AInputComponent.h"
#include "AInteractComponent.h"

void TestAInput(AInputComponent*, Timer&);

bool TestAInit(AInteractComponent*);
void TestAUpdate(AInteractComponent*, Timer&);
void TestADestory(AInteractComponent*);

void DevUsage(SceneNode* _node)
{
    ActorObject a0("a0", *_node);
    ATransformComponent atc0("a0-transform", nullptr);
    AInputComponent aic0("a0-input", nullptr);
    AInteractComponent aitc0("a0-interact", nullptr);

    atc0.ForcePosition({ 11.f,45.f,14.f });
    atc0.ForceRotation({ 20.f,12.f,23.f });
    atc0.ForceScaling({ 2.f,2.f,3.f });
    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    aic0.SetInputFunction(TestAInput);
    a0.AddAComponent(COMP_TYPE::A_INPUT);

    aitc0.SetInitFunction(TestAInit);
    aitc0.SetUpdateFunction(TestAUpdate);
    aitc0.SetDestoryFunction(TestADestory);
    a0.AddAComponent(COMP_TYPE::A_INTERACT);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc0);

    _node->AddActorObject(a0);
}

void TestAInput(AInputComponent* _aic, Timer& _timer)
{
    auto atc = _aic->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    assert(atc);
    float delta = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        atc->TranslateZAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        atc->TranslateXAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        atc->TranslateZAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        atc->TranslateXAsix(0.2f * delta);
    }

    P_LOG(LOG_DEBUG, "%f , %f , %f\n",
        atc->GetProcessingPosition().x,
        atc->GetProcessingPosition().y,
        atc->GetProcessingPosition().z);
}

static int* g_IntArray = nullptr;

bool TestAInit(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "test a init!\n");
    g_IntArray = new int[5];
    for (int i = 0; i < 5; i++) { g_IntArray[i] = i * 2; }

    return true;
}

void TestAUpdate(AInteractComponent*, Timer&)
{
    std::string str = "";
    for (int i = 0; i < 5; i++) { str += std::to_string(g_IntArray[i]); }
    P_LOG(LOG_DEBUG, "%s\n", str.c_str());
}

void TestADestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "test a destory!\n");
    delete[] g_IntArray;
}
