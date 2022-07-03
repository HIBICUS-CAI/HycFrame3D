#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "PhysicsWorld.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-include"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#include "bullet/btBulletCollisionCommon.h"
#pragma clang diagnostic pop

using namespace DirectX;

PhysicsWorld::PhysicsWorld(class SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    mColliedPair({}),
    mContactPointMap({}),
    mCollisionWorld(nullptr),
    mCollisionConfig(nullptr),
    mCollisionDispatcher(nullptr),
    mBroadphaseInterface(nullptr)
{
    CreatePhysicsWorld();
}

PhysicsWorld::~PhysicsWorld()
{

}

void PhysicsWorld::CreatePhysicsWorld()
{
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mCollisionDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mBroadphaseInterface = new btDbvtBroadphase();

    mCollisionWorld = new btCollisionWorld(mCollisionDispatcher,
        mBroadphaseInterface, mCollisionConfig);

#ifdef _DEBUG
    assert(mCollisionConfig && mCollisionDispatcher &&
        mBroadphaseInterface && mCollisionWorld);
#endif // _DEBUG

}

void PhysicsWorld::AddCollisionObject(btCollisionObject* _colliObj)
{
    if (!_colliObj)
    {
        P_LOG(LOG_ERROR, "passing a null collision object to collision world\n");
        return;
    }

    mCollisionWorld->addCollisionObject(_colliObj);
}

void PhysicsWorld::DeleteCollisionObject(btCollisionObject* _colliObj)
{
    if (!_colliObj)
    {
        P_LOG(LOG_ERROR, "searching a null collision object in collision world\n");
        return;
    }

    mCollisionWorld->removeCollisionObject(_colliObj);
}

void PhysicsWorld::DetectCollision()
{
    mColliedPair.clear();
    mContactPointMap.clear();
    mCollisionWorld->performDiscreteCollisionDetection();

    int numManifolds = mCollisionWorld->getDispatcher()->getNumManifolds();

    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = mCollisionWorld->
            getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* obA = (btCollisionObject*)
            (contactManifold->getBody0());
        btCollisionObject* obB = (btCollisionObject*)
            (contactManifold->getBody1());
        int numContacts = contactManifold->getNumContacts();

        COLLIED_PAIR pair = {};
        if (obA < obB) { pair = { obA,obB }; }
        else { pair = { obB,obA }; }

        if (numContacts && mColliedPair.find(pair) == mColliedPair.end())
        {
            mColliedPair.insert(pair);
        }

        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() <= 0.f)
            {
                btVector3 posA = pt.getPositionWorldOnA();
                btVector3 posB = pt.getPositionWorldOnB();
                CONTACT_PONT_PAIR contPair =
                {
                    { posA.getX(),posA.getY(),-posA.getZ() },
                    { posB.getX(),posB.getY(),-posB.getZ() }
                };
                auto found = mContactPointMap.find(pair);
                if (found == mContactPointMap.end())
                {
                    mContactPointMap.insert({ pair,contPair });
                }
                else
                {
                    DirectX::XMVECTOR beforeA =
                        DirectX::XMLoadFloat3(&found->second.first);
                    DirectX::XMVECTOR newoneA =
                        DirectX::XMLoadFloat3(&contPair.first);
                    DirectX::XMVECTOR beforeB =
                        DirectX::XMLoadFloat3(&found->second.second);
                    DirectX::XMVECTOR newoneB =
                        DirectX::XMLoadFloat3(&contPair.second);
                    beforeA += newoneA;
                    beforeB += newoneB;
                    DirectX::XMStoreFloat3(&found->second.first, beforeA);
                    DirectX::XMStoreFloat3(&found->second.second, beforeB);
                }
            }
        }
        if (numContacts)
        {
            auto found = mContactPointMap.find(pair);
            if (found == mContactPointMap.end())
            {
                mColliedPair.erase(pair); continue;
            }
            DirectX::XMVECTOR contactA =
                DirectX::XMLoadFloat3(&found->second.first);
            DirectX::XMVECTOR contactB =
                DirectX::XMLoadFloat3(&found->second.second);
            contactA /= (float)numContacts;
            contactB /= (float)numContacts;
            DirectX::XMStoreFloat3(&found->second.first, contactA);
            DirectX::XMStoreFloat3(&found->second.second, contactB);
        }
    }
}

void PhysicsWorld::DeletePhysicsWorld()
{
    delete mCollisionWorld;
    delete mBroadphaseInterface;
    delete mCollisionDispatcher;
    delete mCollisionConfig;
}

bool PhysicsWorld::CheckCollisionResult(COLLIED_PAIR& _pair,
    CONTACT_PONT_PAIR* _contactPair)
{
    if (mColliedPair.find(_pair) == mColliedPair.end())
    {
        return false;
    }
    else
    {
        if (_contactPair)
        {
            *_contactPair = mContactPointMap.find(_pair)->second;
        }
        return true;
    }
}
