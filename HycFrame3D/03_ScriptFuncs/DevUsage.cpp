#include "DevUsage.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "UiObject.h"

void DevUsage(SceneNode* _node)
{
    ActorObject a0("a0", *_node);
    _node->AddActorObject(a0);
}
