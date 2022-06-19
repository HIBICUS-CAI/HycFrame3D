#pragma once

#include "System.h"
#include <vector>

class RenderSystem :public System
{
public:
    RenderSystem(class SystemExecutive* _sysExecutive);
    virtual ~RenderSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    class RSRoot_DX11* mRenderSystemRoot;
    class AssetsPool* mAssetsPool;
    struct ID3D11ShaderResourceView* mEnvTex;
    struct ID3D11ShaderResourceView* mDiffTex;
    struct ID3D11ShaderResourceView* mSpecTex;
};
