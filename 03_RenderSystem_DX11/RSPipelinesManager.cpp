//---------------------------------------------------------------
// File: RSPipelinesManager.cpp
// Proj: RenderSystem_DX11
// Info: 保存并管理所有的pipeline
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSPipelinesManager.h"
#include "RSRoot_DX11.h"
#include "RSPipeline.h"

#define LOCK EnterCriticalSection(&mDataLock)
#define UNLOCK LeaveCriticalSection(&mDataLock)

RSPipelinesManager::RSPipelinesManager() :
    mRootPtr(nullptr), mCurrentPipeline(nullptr),
    mNextPipeline(nullptr), mPipelineMap({}), mDataLock({})
{

}

RSPipelinesManager::~RSPipelinesManager()
{

}

bool RSPipelinesManager::StartUp(RSRoot_DX11* _root)
{
    if (!_root) { return false; }

    mRootPtr = _root;
    InitializeCriticalSection(&mDataLock);

    return true;
}

void RSPipelinesManager::CleanAndStop()
{
    mNextPipeline = nullptr;
    mCurrentPipeline = nullptr;
    for (auto& pipeline : mPipelineMap)
    {
        pipeline.second->ReleasePipeline();
        delete pipeline.second;
    }
    mPipelineMap.clear();
    DeleteCriticalSection(&mDataLock);
}

void RSPipelinesManager::AddPipeline(
    std::string& _name, RSPipeline* _pipeline)
{
    LOCK;
    if (mPipelineMap.find(_name) == mPipelineMap.end())
    {
        mPipelineMap.insert({ _name,_pipeline });
    }
    UNLOCK;
}

RSPipeline* RSPipelinesManager::GetPipeline(
    std::string& _name)
{
    LOCK;
    auto found = mPipelineMap.find(_name);
    if (found != mPipelineMap.end())
    {
        auto pipeline = found->second;
        UNLOCK;
        return pipeline;
    }
    else
    {
        UNLOCK;
        return nullptr;
    }
}

void RSPipelinesManager::SetPipeline(std::string& _name)
{
    LOCK;
    auto found = mPipelineMap.find(_name);
    if (found != mPipelineMap.end())
    {
        mNextPipeline = (*found).second;
    }
    UNLOCK;
}

void RSPipelinesManager::SetPipeline(RSPipeline* _pipeline)
{
    mNextPipeline = _pipeline;
}

void RSPipelinesManager::ClearCurrentPipelineState()
{
    mCurrentPipeline = nullptr;
    mNextPipeline = nullptr;
}

void RSPipelinesManager::ExecuateCurrentPipeline()
{
    mCurrentPipeline->ExecuatePipeline();
}

void RSPipelinesManager::ProcessNextPipeline()
{
    if (mNextPipeline)
    {
        if (mCurrentPipeline)
        {
            mCurrentPipeline->SuspendAllThread();
        }
        mCurrentPipeline = mNextPipeline;
        mCurrentPipeline->ResumeAllThread();
        mNextPipeline = nullptr;
    }
}
