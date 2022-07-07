//---------------------------------------------------------------
// File: RSPass_Base.h
// Proj: RenderSystem_DX11
// Info: 描述一个抽象的渲染通道并提供基础信息的操作与获取方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 深拷贝的虚函数要加override并返回子类类型的动态分配指针
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSPass_Base {
private:
  const std::string PassName;
  const PASS_TYPE PassType;
  UINT ExecuateOrderInTopic;

  ID3D11Device *Device;
  ID3D11DeviceContext *STContext;
  ID3D11DeviceContext *MTContext;

protected:
  bool HasBeenInited;

public:
  RSPass_Base(const std::string &Name,
              PASS_TYPE Type,
              class RSRoot_DX11 *RootPtr);

  RSPass_Base(const RSPass_Base &Source);

  virtual ~RSPass_Base();

  const std::string &getPassName() const { return PassName; }
  PASS_TYPE getPassType() const { return PassType; }

  void setExecuateOrder(UINT Order) { ExecuateOrderInTopic = Order; }
  UINT getExecuateOrder() const { return ExecuateOrderInTopic; }

  void setMTContext(ID3D11DeviceContext *MTContextPtr) {
    MTContext = MTContextPtr;
  }

  ID3D11Device *device() const { return Device; }
  ID3D11DeviceContext *context() const {
    if (MTContext) {
      return MTContext;
    }
    return STContext;
  }

public:
  virtual RSPass_Base *clonePass() = 0;
  virtual bool initPass() = 0;
  virtual void releasePass() = 0;
  virtual void execuatePass() = 0;
};
