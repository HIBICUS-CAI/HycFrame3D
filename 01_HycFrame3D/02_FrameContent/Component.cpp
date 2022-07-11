#include "Component.h"

Component::Component(const std::string &_compName)
    : ComponentName(_compName), ComponentStatus(STATUS::NEED_INIT) {}

Component::~Component() {}

const std::string &Component::getCompName() const { return ComponentName; }

STATUS Component::getCompStatus() const { return ComponentStatus; }

void Component::setCompStatus(STATUS _compStatus) {
  ComponentStatus = _compStatus;
}
