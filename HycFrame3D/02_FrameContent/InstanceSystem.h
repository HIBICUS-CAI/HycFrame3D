#pragma once

#include "System.h"

#include <vector>

class InstanceSystem : public System {
private:
  std::vector<class ATransformComponent> *ATransArrayPtr;
  std::vector<class UTransformComponent> *UTransArrayPtr;
  std::vector<class AMeshComponent> *AMeshArrayPtr;
  std::vector<class ALightComponent> *ALightArrayPtr;
  std::vector<class AParticleComponent> *AParitcleArrayPtr;
  std::vector<class ASpriteComponent> *ASpriteArrayPtr;
  std::vector<class USpriteComponent> *USpriteArrayPtr;
  std::vector<class UAnimateComponent> *UAnimateArrayPtr;

public:
  InstanceSystem(class SystemExecutive *SysExecutive);
  virtual ~InstanceSystem();

public:
  virtual bool init();
  virtual void run(Timer &Timer);
  virtual void destory();
};
