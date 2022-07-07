//---------------------------------------------------------------
// File: RSPipeline.h
// Proj: RenderSystem_DX11
// Info: 描述一个具体的流水线并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建流水线时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

struct TOPIC_THREAD {
  ID3D11DeviceContext *DeferredContext = nullptr;
  ID3D11CommandList *CommandList = nullptr;
  HANDLE ThreadHandle = nullptr;
  HANDLE BeginEvent = nullptr;
  HANDLE FinishEvent = nullptr;
  bool ExitFlag = false;

  struct ARGUMENT_LIST {
    class RSTopic *TopicPtr = nullptr;
    HANDLE BeginEventPtr = nullptr;
    HANDLE FinishEventPtr = nullptr;
    bool *ExitFlagPtr = nullptr;
    ID3D11DeviceContext *DeferredContext = nullptr;
    ID3D11CommandList **CommandListPtr = nullptr;
  } ArgumentList = {};
};

class RSPipeline {
private:
  const std::string PipelineName;
  bool AssemblyFinishFlag;

  std::vector<class RSTopic *> TopicArray;
  std::vector<TOPIC_THREAD> TopicThreads;
  std::vector<HANDLE> FinishEvents;

  ID3D11DeviceContext *ImmediateContext;
  bool MultipleThreadModeFlag;

public:
  RSPipeline(const std::string &_name);
  RSPipeline(const RSPipeline &_source);
  ~RSPipeline();

  const std::string &getPipelineName() const;

  void startAssembly();
  void finishAssembly();

  bool hasTopic(const std::string &TopicName);
  void insertTopic(class RSTopic *Topic);
  void eraseTopic(class RSTopic *Topic);
  void eraseTopic(const std::string &TopicName);

  bool initAllTopics(class RSDevices *DevicesPtr,
                     bool ForceSingleThreadFlag = false);

  void execuatePipeline();
  void releasePipeline();

  void suspendAllThread();
  void resumeAllThread();
};
