#include "PhysicsWorld.h"
#include "bullet/btBulletCollisionCommon.h"

PhysicsWorld::PhysicsWorld(class SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode), mCollisionWorld(nullptr),
    mCollisionConfig(nullptr), mCollisionDispatcher(nullptr),
    mBroadphaseInterface(nullptr), mColliedPair({})
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

        COLLIED_PAIR pair0 = { obA,obB };
        COLLIED_PAIR pair1 = { obB,obA };

        if (mColliedPair.find(pair0) == mColliedPair.end() &&
            mColliedPair.find(pair1) == mColliedPair.end())
        {
            mColliedPair.insert(pair0);
        }

        /*int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() <= 0.f)
            {
                btVector3 posA = pt.getPositionWorldOnA();
                btVector3 posB = pt.getPositionWorldOnB();
            }
        }*/
    }
}

void PhysicsWorld::DeletePhysicsWorld()
{
    delete mCollisionWorld;
    delete mBroadphaseInterface;
    delete mCollisionDispatcher;
    delete mCollisionConfig;
}

bool PhysicsWorld::CheckCollisionResult(
    COLLIED_PAIR& _pair0, COLLIED_PAIR& _pair1)
{
    if (mColliedPair.find(_pair0) == mColliedPair.end() &&
        mColliedPair.find(_pair1) == mColliedPair.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}
