//---------------------------------------------------------------
// File: RSPipeline.h
// Proj: RenderSystem_VK
// Info: 描述一个具体的流水线并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建流水线时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

class RSPipeline {
private:
  const std::string PipelineName;
  bool AssemblyFinishFlag;

  std::vector<class RSTopic *> TopicArray;
  // one command context per topic, submitted in topic order
  std::vector<class RSCommandContext *> TopicContexts;
  class RSDevices *Devices;
  bool MultipleThreadModeFlag;

  // workers record topics in parallel, one thread per topic
  std::vector<std::thread> TopicThreads;
  std::mutex ThreadLock;
  std::condition_variable BeginCondition;
  std::condition_variable FinishCondition;
  uint64_t WorkGeneration;
  size_t RemainingWork;
  bool ExitFlag;

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

  // threads only run when the pipeline executes, nothing to suspend
  void suspendAllThread();
  void resumeAllThread();

private:
  void topicThreadFunc(size_t TopicIndex);
};
