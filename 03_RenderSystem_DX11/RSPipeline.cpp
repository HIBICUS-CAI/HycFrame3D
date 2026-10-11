//---------------------------------------------------------------
// File: RSPipeline.cpp
// Proj: RenderSystem_VK
// Info: 描述一个具体的流水线并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建流水线时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#include "RSPipeline.h"

#include "RSCommandContext.h"
#include "RSDevices.h"
#include "RSTopic.h"

#include <algorithm>
#include <cassert>

RSPipeline::RSPipeline(const std::string &Name)
    : PipelineName(Name), AssemblyFinishFlag(true), TopicArray({}),
      TopicContexts({}), Devices(nullptr), MultipleThreadModeFlag(false),
      TopicThreads(), ThreadLock(), BeginCondition(), FinishCondition(),
      WorkGeneration(0), RemainingWork(0), ExitFlag(false) {}

RSPipeline::RSPipeline(const RSPipeline &Source)
    : PipelineName(Source.PipelineName),
      AssemblyFinishFlag(Source.AssemblyFinishFlag), TopicArray({}),
      TopicContexts({}), Devices(Source.Devices),
      MultipleThreadModeFlag(Source.MultipleThreadModeFlag), TopicThreads(),
      ThreadLock(), BeginCondition(), FinishCondition(), WorkGeneration(0),
      RemainingWork(0), ExitFlag(false) {
  TopicArray.reserve(Source.TopicArray.size());
  for (auto &Topic : Source.TopicArray) {
    RSTopic *OneMore = new RSTopic(*Topic);
    TopicArray.push_back(OneMore);
  }
}

RSPipeline::~RSPipeline() {}

const std::string &RSPipeline::getPipelineName() const { return PipelineName; }

void RSPipeline::startAssembly() { AssemblyFinishFlag = false; }

void RSPipeline::finishAssembly() { AssemblyFinishFlag = true; }

bool RSPipeline::hasTopic(const std::string &TopicName) {
  for (auto &Topic : TopicArray) {
    if (Topic->getTopicName() == TopicName) {
      return true;
    }
  }

  return false;
}

bool topicExecLessCompare(const RSTopic *A, const RSTopic *B) {
  return A->getExecuateOrder() < B->getExecuateOrder();
}

void RSPipeline::insertTopic(RSTopic *Topic) {
  if (!AssemblyFinishFlag) {
    TopicArray.push_back(Topic);
    std::sort(TopicArray.begin(), TopicArray.end(), topicExecLessCompare);
  }
}

void RSPipeline::eraseTopic(RSTopic *Topic) {
  if (!AssemblyFinishFlag) {
    for (auto I = TopicArray.begin(), E = TopicArray.end(); I != E; I++) {
      if (*I == Topic) {
        (*I)->releaseTopic();
        delete (*I);
        TopicArray.erase(I);
        return;
      }
    }
  }
}

void RSPipeline::eraseTopic(const std::string &TopicName) {
  if (!AssemblyFinishFlag) {
    for (auto I = TopicArray.begin(), E = TopicArray.end(); I != E; I++) {
      if ((*I)->getTopicName() == TopicName) {
        (*I)->releaseTopic();
        delete (*I);
        TopicArray.erase(I);
        return;
      }
    }
  }
}

bool RSPipeline::initAllTopics(RSDevices *DevicesPtr,
                               bool ForceSingleThreadFlag) {
  if (!DevicesPtr) {
    return false;
  }
  Devices = DevicesPtr;
  MultipleThreadModeFlag = DevicesPtr->getCommandListSupport();
  MultipleThreadModeFlag = MultipleThreadModeFlag && (!ForceSingleThreadFlag);

  if (!AssemblyFinishFlag) {
    return false;
  }

  for (auto &Topic : TopicArray) {
    auto Context = new RSCommandContext();
    if (!Context->startUp(Devices)) {
      delete Context;
      return false;
    }
    TopicContexts.push_back(Context);
    Topic->setContext(Context);
    if (!Topic->initAllPasses()) {
      return false;
    }
  }

  if (MultipleThreadModeFlag) {
    for (size_t I = 0; I < TopicArray.size(); I++) {
      TopicThreads.emplace_back(&RSPipeline::topicThreadFunc, this, I);
    }
  }

  return true;
}

void RSPipeline::execuatePipeline() {
  if (!AssemblyFinishFlag) {
    return;
  }

  Devices->beginFrame();

  if (MultipleThreadModeFlag) {
    std::unique_lock<std::mutex> Lock(ThreadLock);
    RemainingWork = TopicArray.size();
    ++WorkGeneration;
    BeginCondition.notify_all();
    FinishCondition.wait(Lock, [this]() { return RemainingWork == 0; });
  } else {
    for (size_t I = 0; I < TopicArray.size(); I++) {
      TopicContexts[I]->beginRecording();
      TopicArray[I]->execuateTopic();
      TopicContexts[I]->endRecording();
    }
  }

  // gpu executes topics in the same order as the original command lists
  for (auto Context : TopicContexts) {
    Devices->submitContext(Context);
  }
}

void RSPipeline::releasePipeline() {
  if (!AssemblyFinishFlag) {
    return;
  }

  {
    std::lock_guard<std::mutex> Lock(ThreadLock);
    ExitFlag = true;
    BeginCondition.notify_all();
  }
  for (auto &Thread : TopicThreads) {
    if (Thread.joinable()) {
      Thread.join();
    }
  }
  TopicThreads.clear();

  for (auto &Topic : TopicArray) {
    Topic->releaseTopic();
    delete Topic;
  }
  TopicArray.clear();

  for (auto Context : TopicContexts) {
    Context->cleanAndStop();
    delete Context;
  }
  TopicContexts.clear();
}

void RSPipeline::topicThreadFunc(size_t TopicIndex) {
  uint64_t SeenGeneration = 0;
  while (true) {
    {
      std::unique_lock<std::mutex> Lock(ThreadLock);
      BeginCondition.wait(Lock, [this, SeenGeneration]() {
        return ExitFlag || WorkGeneration != SeenGeneration;
      });
      if (ExitFlag) {
        break;
      }
      SeenGeneration = WorkGeneration;
    }

    TopicContexts[TopicIndex]->beginRecording();
    TopicArray[TopicIndex]->execuateTopic();
    TopicContexts[TopicIndex]->endRecording();

    {
      std::lock_guard<std::mutex> Lock(ThreadLock);
      if (--RemainingWork == 0) {
        FinishCondition.notify_all();
      }
    }
  }
}

void RSPipeline::suspendAllThread() {}

void RSPipeline::resumeAllThread() {}
