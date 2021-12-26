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

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    _node->AddActorObject(a0);
}
