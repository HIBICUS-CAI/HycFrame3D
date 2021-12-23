#include "Component.h"

Component::Component(std::string&& _compName) :
    mComponentName(_compName), mComponentStatus(STATUS::NEED_INIT)
{

}

Component::Component(std::string& _compName) :
    mComponentName(_compName), mComponentStatus(STATUS::NEED_INIT)
{

}

Component::~Component()
{

}

const std::string& Component::GetCompName() const
{
    return mComponentName;
}

STATUS Component::GetCompStatus() const
{
    return mComponentStatus;
}

void Component::SetCompStatus(STATUS _compStatus)
{

}
