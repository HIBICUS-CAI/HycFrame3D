#include "ACollisionComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "PhysicsWorld.h"
#include "ATransformComponent.h"
#include "bullet/btBulletCollisionCommon.h"

ACollisionComponent::ACollisionComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mCollisionObject(nullptr), mCollisionShape(nullptr)
{

}

ACollisionComponent::ACollisionComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mCollisionObject(nullptr), mCollisionShape(nullptr)
{

}

ACollisionComponent::~ACollisionComponent()
{

}

bool ACollisionComponent::Init()
{
    if (!mCollisionObject || !mCollisionShape)
    {
        P_LOG(LOG_ERROR, "invalid collision object in %s\n",
            GetCompName().c_str());
        return false;
    }

    AddCollisionObjectToWorld();

    return true;
}

void ACollisionComponent::Update(Timer& _timer)
{
    SyncDataFromTransform();
}

void ACollisionComponent::Destory()
{
    GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        DeleteCollisionObject(mCollisionObject);
    delete mCollisionObject;
    delete mCollisionShape;
}

bool ACollisionComponent::CheckCollisionWith(std::string&& _actorName)
{
    auto actor = GetActorOwner()->GetSceneNode().GetActorObject(_actorName);
    if (!actor)
    {
        P_LOG(LOG_ERROR, "doesnt exist a actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    auto acc = actor->GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    if (!acc)
    {
        P_LOG(LOG_ERROR, "doesnt exist a collision comp in actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    COLLIED_PAIR pair0 = { mCollisionObject,acc->GetCollisionObject() };
    COLLIED_PAIR pair1 = { pair0.second,pair0.first };

    return GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        CheckCollisionResult(pair0, pair1);
}

bool ACollisionComponent::CheckCollisionWith(std::string& _actorName)
{
    auto actor = GetActorOwner()->GetSceneNode().GetActorObject(_actorName);
    if (!actor)
    {
        P_LOG(LOG_ERROR, "doesnt exist a actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    auto acc = actor->GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    if (!acc)
    {
        P_LOG(LOG_ERROR, "doesnt exist a collision comp in actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    COLLIED_PAIR pair0 = { mCollisionObject,acc->GetCollisionObject() };
    COLLIED_PAIR pair1 = { pair0.second,pair0.first };

    return GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        CheckCollisionResult(pair0, pair1);
}

void ACollisionComponent::CreateCollisionShape(COLLISION_SHAPE _type,
    DirectX::XMFLOAT3 _size)
{
    switch (_type)
    {
    case COLLISION_SHAPE::BOX:
        mCollisionShape = new btBoxShape(
            { _size.x / 2.f,_size.y / 2.f,_size.z / 2.f });
        break;
    case COLLISION_SHAPE::SPHERE:
        mCollisionShape = new btSphereShape({ _size.x });
        break;
    default:
        assert(mCollisionShape);
        break;
    }
    mCollisionObject = new btCollisionObject();
    mCollisionObject->setCollisionShape(mCollisionShape);
}

void ACollisionComponent::AddCollisionObjectToWorld()
{
    GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        AddCollisionObject(mCollisionObject);
}

void ACollisionComponent::SyncDataFromTransform()
{
    ATransformComponent* atc = GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
#ifdef _DEBUG
    assert(atc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = atc->GetProcessingPosition();
    DirectX::XMFLOAT3 angle = atc->GetProcessingRotation();

    btTransform trans = {};
    trans.setIdentity();
    trans.setOrigin(btVector3(world.x, world.y, -world.z));
    trans.setRotation(btQuaternion(angle.y, angle.x, -angle.z));
    mCollisionObject->setWorldTransform(trans);
}

btCollisionObject* ACollisionComponent::GetCollisionObject() const
{
    return mCollisionObject;
}
