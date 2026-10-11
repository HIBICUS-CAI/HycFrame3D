//---------------------------------------------------------------
// File: RSPass_Base.cpp
// Proj: RenderSystem_VK
// Info: 描述一个抽象的渲染通道并提供基础信息的操作与获取方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 深拷贝的虚函数要加override并返回子类类型的动态分配指针
//---------------------------------------------------------------

#include "RSPass_Base.h"

#include "RSDevices.h"
#include "RSRoot_VK.h"

RSPass_Base::RSPass_Base(const std::string &Name,
                         PASS_TYPE Type,
                         RSRoot_VK *RootPtr)
    : PassName(Name), PassType(Type), ExecuateOrderInTopic(RS_INVALID_ORDER),
      Devices(RootPtr->getDevices()), Context(nullptr), HasBeenInited(false) {}

RSPass_Base::RSPass_Base(const RSPass_Base &_source)
    : PassName(_source.PassName), PassType(_source.PassType),
      ExecuateOrderInTopic(_source.ExecuateOrderInTopic),
      Devices(_source.Devices), Context(_source.Context),
      HasBeenInited(_source.HasBeenInited) {}

RSPass_Base::~RSPass_Base() {}
