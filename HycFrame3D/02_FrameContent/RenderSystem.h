#pragma once

#include "System.h"

#include <vector>

class RenderSystem : public System {
private:
  class RSRoot_DX11 *RenderSystemRoot;
  class AssetsPool *AssetsPool;
  struct ID3D11ShaderResourceView *IblEnvTex;
  struct ID3D11ShaderResourceView *IblDiffTex;
  struct ID3D11ShaderResourceView *IblSpecTex;

public:
  RenderSystem(class SystemExecutive *SysExecutive);
  virtual ~RenderSystem();

public:
  virtual bool init();
  virtual void run(Timer &Timer);
  virtual void destory();
};
