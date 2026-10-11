#pragma once

#include "System.h"

#include <vector>

class RenderSystem : public System {
private:
  class RSRoot_VK *RenderSystemRoot;
  class AssetsPool *AssetsPool;
  class RSShaderResourceView *IblEnvTex;
  class RSShaderResourceView *IblDiffTex;
  class RSShaderResourceView *IblSpecTex;

public:
  RenderSystem(class SystemExecutive *SysExecutive);
  virtual ~RenderSystem();

public:
  virtual bool init();
  virtual void run(const Timer &Timer);
  virtual void destory();
};
