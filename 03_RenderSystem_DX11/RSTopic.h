//---------------------------------------------------------------
// File: RSTopic.h
// Proj: RenderSystem_DX11
// Info: 描述一个具体的主题并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建主题时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSTopic {
private:
  const std::string TopicName;
  bool AssemblyFinishFlag;
  UINT ExecuateOrderInPipeline;
  std::vector<class RSPass_Base *> PassArray;
  ID3D11DeviceContext *MTContext;

public:
  RSTopic(const std::string &Name);

  RSTopic(const RSTopic &Source);

  ~RSTopic();

  const std::string &getTopicName() const;

  void startAssembly();
  void finishAssembly();

  void setExecuateOrder(UINT Order);
  UINT getExecuateOrder() const;

  void setMTContext(ID3D11DeviceContext *ContextPtr);

  void insertPass(class RSPass_Base *Pass);
  void erasePass(class RSPass_Base *Pass);
  void erasePass(const std::string &PassName);
  bool hasPass(const std::string &PassName);

  bool initAllPasses();
  void execuateTopic();
  void releaseTopic();
};
