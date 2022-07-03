#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "ACollisionComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "PhysicsWorld.h"
#include "ATransformComponent.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-include"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#include "bullet/btBulletCollisionCommon.h"
#pragma clang diagnostic pop

using namespace DirectX;

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

bool ACollisionComponent::CheckCollisionWith(std::string&& _actorName,
    CONTACT_PONT_PAIR* _contactPair)
{
    auto actor = GetActorOwner()->GetSceneNode().GetActorObject(_actorName);
    if (!actor)
    {
        P_LOG(LOG_ERROR, "doesnt exist a actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    auto acc = actor->GetComponent<ACollisionComponent>();
    if (!acc)
    {
        P_LOG(LOG_ERROR, "doesnt exist a collision comp in actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    COLLIED_PAIR pair = {};
    if (mCollisionObject < acc->GetCollisionObject())
    {
        pair = { mCollisionObject,acc->GetCollisionObject() };
    }
    else
    {
        pair = { acc->GetCollisionObject(),mCollisionObject };
    }

    return GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        CheckCollisionResult(pair, _contactPair);
}

bool ACollisionComponent::CheckCollisionWith(std::string& _actorName,
    CONTACT_PONT_PAIR* _contactPair)
{
    auto actor = GetActorOwner()->GetSceneNode().GetActorObject(_actorName);
    if (!actor)
    {
        P_LOG(LOG_ERROR, "doesnt exist a actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    auto acc = actor->GetComponent<ACollisionComponent>();
    if (!acc)
    {
        P_LOG(LOG_ERROR, "doesnt exist a collision comp in actor name : %s\n",
            _actorName.c_str());
        return false;
    }

    COLLIED_PAIR pair = {};
    if (mCollisionObject < acc->GetCollisionObject())
    {
        pair = { mCollisionObject,acc->GetCollisionObject() };
    }
    else
    {
        pair = { acc->GetCollisionObject(),mCollisionObject };
    }

    return GetActorOwner()->GetSceneNode().GetPhysicsWorld()->
        CheckCollisionResult(pair, _contactPair);
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
        mCollisionShape = new btSphereShape(_size.x);
        break;
    default:
        assert(mCollisionShape);
        break;
    }
    mCollisionShape->setMargin(0.f);
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
        GetComponent<ATransformComponent>();
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

DirectX::XMFLOAT3 ACollisionComponent::CalcCenterOfContact(
    CONTACT_PONT_PAIR& _pair)
{
    DirectX::XMFLOAT3 center = {};
    DirectX::XMVECTOR contactA = DirectX::XMLoadFloat3(&_pair.first);
    DirectX::XMVECTOR contactB = DirectX::XMLoadFloat3(&_pair.second);
    DirectX::XMVECTOR centerV = {};
    centerV = (contactA + contactB) / 2.f;
    DirectX::XMStoreFloat3(&center, centerV);
    return center;
}
