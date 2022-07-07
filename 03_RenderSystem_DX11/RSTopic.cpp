//---------------------------------------------------------------
// File: RSTopic.cpp
// Proj: RenderSystem_DX11
// Info: 描述一个具体的主题并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建主题时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#include "RSTopic.h"

#include "RSPass_Base.h"

#include <algorithm>

RSTopic::RSTopic(const std::string &Name)
    : TopicName(Name), AssemblyFinishFlag(true),
      ExecuateOrderInPipeline(RS_INVALID_ORDER), PassArray({}),
      MTContext(nullptr) {}

RSTopic::RSTopic(const RSTopic &Source)
    : TopicName(Source.TopicName),
      AssemblyFinishFlag(Source.AssemblyFinishFlag),
      ExecuateOrderInPipeline(Source.ExecuateOrderInPipeline), PassArray({}),
      MTContext(Source.MTContext) {
  PassArray.reserve(Source.PassArray.size());
  for (auto &Topic : Source.PassArray) {
    RSPass_Base *OneMore = Topic->clonePass();
    PassArray.push_back(OneMore);
  }
}

RSTopic::~RSTopic() {}

const std::string &RSTopic::getTopicName() const { return TopicName; }

void RSTopic::startAssembly() { AssemblyFinishFlag = false; }

void RSTopic::finishAssembly() { AssemblyFinishFlag = true; }

void RSTopic::setExecuateOrder(UINT Order) { ExecuateOrderInPipeline = Order; }

UINT RSTopic::getExecuateOrder() const { return ExecuateOrderInPipeline; }

void RSTopic::setMTContext(ID3D11DeviceContext *ContextPtr) {
  MTContext = ContextPtr;
  for (auto &Pass : PassArray) {
    Pass->setMTContext(ContextPtr);
  }
}

bool passExecLessCompare(const RSPass_Base *A, const RSPass_Base *B) {
  return A->getExecuateOrder() < B->getExecuateOrder();
}

void RSTopic::insertPass(RSPass_Base *Pass) {
  if (!AssemblyFinishFlag) {
    PassArray.push_back(Pass);
    std::sort(PassArray.begin(), PassArray.end(), passExecLessCompare);
  }
}

void RSTopic::erasePass(RSPass_Base *Pass) {
  if (!AssemblyFinishFlag) {
    for (auto I = PassArray.begin(), E = PassArray.end(); I != E; I++) {
      if (*I == Pass) {
        (*I)->releasePass();
        delete (*I);
        PassArray.erase(I);
        return;
      }
    }
  }
}

void RSTopic::erasePass(const std::string &PassName) {
  if (!AssemblyFinishFlag) {
    for (auto I = PassArray.begin(), E = PassArray.end(); I != E; I++) {
      if ((*I)->getPassName() == PassName) {
        (*I)->releasePass();
        delete (*I);
        PassArray.erase(I);
        return;
      }
    }
  }
}

bool RSTopic::hasPass(const std::string &PassName) {
  for (auto &Pass : PassArray) {
    if (Pass->getPassName() == PassName) {
      return true;
    }
  }

  return false;
}

bool RSTopic::initAllPasses() {
  if (AssemblyFinishFlag) {
    for (auto &Pass : PassArray) {
      if (!Pass->initPass()) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

void RSTopic::execuateTopic() {
  if (AssemblyFinishFlag) {
    for (auto &Pass : PassArray) {
      Pass->execuatePass();
    }
  }
}

void RSTopic::releaseTopic() {
  if (AssemblyFinishFlag) {
    for (auto &Pass : PassArray) {
      Pass->releasePass();
      delete Pass;
    }
    PassArray.clear();
  }
}
