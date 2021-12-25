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
    a0.AddAComponent(COMP_TYPE::U_ANIMATE);
    // TODO 先到容器中生成，然后再将初始化完成的obj引用传过去

    _node->AddActorObject(a0);

    ATransformComponent atc0("atc0", a0);
    ATransformComponent atc1("atc1", a0);
    UiObject u0("u0", *_node);
    UAnimateComponent uac0("uac0", u0);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::U_ANIMATE, uac0);
    _node->GetComponentContainer()->
        DeleteComponent(COMP_TYPE::A_TRANSFORM, "atc0");
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc1);
}
