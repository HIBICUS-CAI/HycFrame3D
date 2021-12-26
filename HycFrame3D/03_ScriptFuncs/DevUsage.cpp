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

    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);
    // TODO �ȵ����������ɣ�Ȼ���ٽ���ʼ����ɵ�obj���ô���ȥ

    _node->AddActorObject(a0);

    ATransformComponent atc0("a0-transform", nullptr);
    ATransformComponent atc1("a0-transform", nullptr);
    UiObject u0("u0", *_node);
    u0.AddUComponent(COMP_TYPE::U_ANIMATE);
    _node->AddUiObject(u0);

    UAnimateComponent uac0("u0-animate", nullptr);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::U_ANIMATE, uac0);
    _node->GetComponentContainer()->
        DeleteComponent(COMP_TYPE::A_TRANSFORM, "a0-transform");
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc1);
}
