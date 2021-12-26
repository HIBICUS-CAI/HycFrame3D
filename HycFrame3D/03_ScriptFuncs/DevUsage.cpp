#include "DevUsage.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "UiObject.h"
#include "ID_Interface.h"
#include "ComponentContainer.h"
#include "ATransformComponent.h"
#include "UAnimateComponent.h"
#include "AInputComponent.h"

void TestAInput(AInputComponent*, Timer&);

void DevUsage(SceneNode* _node)
{
    ActorObject a0("a0", *_node);
    ATransformComponent atc0("a0-transform", nullptr);
    AInputComponent aic0("a0-input", nullptr);

    atc0.ForcePosition({ 11.f,45.f,14.f });
    atc0.ForceRotation({ 20.f,12.f,23.f });
    atc0.ForceScaling({ 2.f,2.f,3.f });
    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    aic0.SetInputFunction(TestAInput);
    a0.AddAComponent(COMP_TYPE::A_INPUT);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);

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
