#include "SPInput.h"

void RegisterSPInput(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TestASpInput),TestASpInput });
}

void TestASpInput(AInputComponent* _aic, Timer& _timer)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test2", "test2");
    }
}
