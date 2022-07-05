//---------------------------------------------------------------
// File: RSPipeline.cpp
// Proj: RenderSystem_DX11
// Info: 描述一个具体的流水线并提供执行与操作方式
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 创建流水线时一定不能直接使用指针，一定要新构造一个对象
//---------------------------------------------------------------

#include "RSPipeline.h"

#include "RSDevices.h"
#include "RSTopic.h"

#include <algorithm>
#include <process.h>
#include <thread>

unsigned __stdcall topicThreadFunc(PVOID Args);

RSPipeline::RSPipeline(const std::string &Name)
    : PipelineName(Name), AssemblyFinishFlag(true), TopicArray({}),
      TopicThreads({}), FinishEvents({}), ImmediateContext(nullptr),
      MultipleThreadModeFlag(false) {}

RSPipeline::RSPipeline(const RSPipeline &Source)
    : PipelineName(Source.PipelineName),
      AssemblyFinishFlag(Source.AssemblyFinishFlag), TopicArray({}),
      TopicThreads({}), FinishEvents({}),
      ImmediateContext(Source.ImmediateContext),
      MultipleThreadModeFlag(Source.MultipleThreadModeFlag) {
  TopicArray.reserve(Source.TopicArray.size());
  for (auto &Topic : Source.TopicArray) {
    RSTopic *OneMore = new RSTopic(*Topic);
    TopicArray.push_back(OneMore);
  }
}

RSPipeline::~RSPipeline() {}

const std::string &
RSPipeline::getPipelineName() const {
  return PipelineName;
}

void
RSPipeline::startAssembly() {
  AssemblyFinishFlag = false;
}

void
RSPipeline::finishAssembly() {
  AssemblyFinishFlag = true;
}

bool
RSPipeline::hasTopic(const std::string &TopicName) {
  for (auto &Topic : TopicArray) {
    if (Topic->getTopicName() == TopicName) {
      return true;
    }
  }

  return false;
}

bool
topicExecLessCompare(const RSTopic *A, const RSTopic *B) {
  return A->getExecuateOrder() < B->getExecuateOrder();
}

void
RSPipeline::insertTopic(RSTopic *Topic) {
  if (!AssemblyFinishFlag) {
    TopicArray.push_back(Topic);
    std::sort(TopicArray.begin(), TopicArray.end(), topicExecLessCompare);
  }
}

void
RSPipeline::eraseTopic(RSTopic *Topic) {
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

void
RSPipeline::eraseTopic(const std::string &TopicName) {
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

bool
RSPipeline::initAllTopics(RSDevices *DevicesPtr, bool ForceSingleThreadFlag) {
  if (!DevicesPtr) {
    return false;
  }
  ImmediateContext = DevicesPtr->GetSTContext();
  MultipleThreadModeFlag = DevicesPtr->GetCommandListSupport();
  MultipleThreadModeFlag = MultipleThreadModeFlag && (!ForceSingleThreadFlag);

  if (AssemblyFinishFlag) {
    for (auto &Topic : TopicArray) {
      if (!Topic->initAllPasses()) {
        return false;
      }
      if (MultipleThreadModeFlag) {
        ID3D11DeviceContext *Deferred = nullptr;
        HRESULT Hr =
            DevicesPtr->GetDevice()->CreateDeferredContext(0, &Deferred);
        FAIL_HR_RETURN(Hr);
        Topic->setMTContext(Deferred);

        TOPIC_THREAD TT = {};
        TT.DeferredContext = Deferred;
        TT.CommandList = nullptr;
        TT.ThreadHandle = NULL;
        TT.ExitFlag = false;
        TT.ArgumentList.TopicPtr = Topic;
        TT.ArgumentList.DeferredContext = Deferred;
        TopicThreads.emplace_back(TT);
      }
    }

    UINT CoreCount = std::thread::hardware_concurrency();
    DWORD_PTR Affinity = 0;
    DWORD_PTR Mask = 0;
    for (auto &TT : TopicThreads) {
      TT.ArgumentList.ExitFlagPtr = &TT.ExitFlag;
      TT.ArgumentList.CommandListPtr = &TT.CommandList;

      TT.BeginEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      TT.FinishEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      FinishEvents.emplace_back(TT.FinishEvent);

      TT.ArgumentList.BeginEventPtr = TT.BeginEvent;
      TT.ArgumentList.FinishEventPtr = TT.FinishEvent;

      TT.ThreadHandle = reinterpret_cast<HANDLE>(
          _beginthreadex(nullptr, 0, topicThreadFunc, &(TT.ArgumentList),
                         CREATE_SUSPENDED, nullptr));
      if (!TT.ThreadHandle) {
        return false;
      }
      Mask = SetThreadAffinityMask(TT.ThreadHandle, static_cast<DWORD_PTR>(1)
                                                        << Affinity);
      Affinity = (++Affinity) % CoreCount;
    }
    (void)Mask;

    return true;
  } else {
    return false;
  }
}

void
RSPipeline::execuatePipeline() {
  if (AssemblyFinishFlag) {
    if (MultipleThreadModeFlag) {
      for (auto &TT : TopicThreads) {
        SetEvent(TT.BeginEvent);
      }

      WaitForMultipleObjects(static_cast<DWORD>(FinishEvents.size()),
                             &FinishEvents[0], TRUE, INFINITE);

      for (auto &TT : TopicThreads) {
        if (!TT.CommandList) {
          assert(false);
          return;
        }
        ImmediateContext->ExecuteCommandList(TT.CommandList, FALSE);
        SAFE_RELEASE(TT.CommandList);
      }
    } else {
      for (auto &Topic : TopicArray) {
        Topic->execuateTopic();
      }
    }
  }
}

void
RSPipeline::releasePipeline() {
  if (AssemblyFinishFlag) {
    for (auto &Topic : TopicArray) {
      Topic->releaseTopic();
      delete Topic;
    }
    TopicArray.clear();
    if (MultipleThreadModeFlag) {
      std::vector<HANDLE> HandleArray = {};
      for (auto &TT : TopicThreads) {
        HandleArray.emplace_back(TT.ThreadHandle);
        TT.ExitFlag = true;
      }
    }
    for (auto &TT : TopicThreads) {
      SAFE_RELEASE(TT.DeferredContext);
      SAFE_RELEASE(TT.CommandList);
      CloseHandle(TT.BeginEvent);
      CloseHandle(TT.FinishEvent);
    }
  }
}

unsigned __stdcall topicThreadFunc(PVOID Args) {
  TOPIC_THREAD::ARGUMENT_LIST *Argument = nullptr;
  Argument = static_cast<TOPIC_THREAD::ARGUMENT_LIST *>(Args);
  if (!Argument) {
    return -1;
  }

  auto Topic = Argument->TopicPtr;
  auto Deferred = Argument->DeferredContext;
  auto ListPtr = Argument->CommandListPtr;
  HANDLE Begin = Argument->BeginEventPtr;
  HANDLE Finish = Argument->FinishEventPtr;
  auto ExitFlagPtr = Argument->ExitFlagPtr;

  HRESULT Hr = S_OK;
  while (true) {
    WaitForSingleObject(Begin, INFINITE);
    if (*ExitFlagPtr) {
      break;
    }
    Topic->execuateTopic();
    Hr = Deferred->FinishCommandList(FALSE, ListPtr);
#ifdef _DEBUG
    if (FAILED(Hr)) {
      return -2;
    }
#endif // _DEBUG
    SetEvent(Finish);
  }
  (void)Hr;

  return 0;
}

void
RSPipeline::suspendAllThread() {
  for (auto &TT : TopicThreads) {
    SuspendThread(TT.ThreadHandle);
  }
}

void
RSPipeline::resumeAllThread() {
  for (auto &TT : TopicThreads) {
    ResumeThread(TT.ThreadHandle);
  }
}
