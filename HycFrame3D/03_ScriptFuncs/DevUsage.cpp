#include "DevUsage.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "UiObject.h"
#include "ComponentContainer.h"
#include "ATransformComponent.h"
#include "UAnimateComponent.h"

void DevUsage(SceneNode* _node)
{
    ActorObject a0("a0", *_node);
    ATransformComponent atc0("a0-transform", nullptr);

    atc0.ForcePosition({ 11.f,45.f,14.f });
    atc0.ForceRotation({ 20.f,12.f,23.f });
    atc0.ForceScaling({ 2.f,2.f,3.f });
    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);

    _node->AddActorObject(a0);
}
