#pragma once

#include "Hyc3DCommon.h"
#include <string>

namespace ComponentGetter
{
    template<typename T>
    inline void GenerateCompName(std::string& _outName)
    {
        _outName = "this comp type doesn't exist";
    }

    template<>
    inline void GenerateCompName<class ATransformComponent>(std::string& _outName)
    {
        _outName += "-transform";
    }

    template<>
    inline void GenerateCompName<class AInputComponent>(std::string& _outName)
    {
        _outName += "-input";
    }

    template<>
    inline void GenerateCompName<class AInteractComponent>(std::string& _outName)
    {
        _outName += "-interact";
    }

    template<>
    inline void GenerateCompName<class ATimerComponent>(std::string& _outName)
    {
        _outName += "-timer";
    }

    template<>
    inline void GenerateCompName<class ACollisionComponent>(std::string& _outName)
    {
        _outName += "-collision";
    }

    template<>
    inline void GenerateCompName<class ASpriteComponent>(std::string& _outName)
    {
        _outName += "-sprite";
    }

    template<>
    inline void GenerateCompName<class AMeshComponent>(std::string& _outName)
    {
        _outName += "-mesh";
    }

    template<>
    inline void GenerateCompName<class ALightComponent>(std::string& _outName)
    {
        _outName += "-light";
    }

    template<>
    inline void GenerateCompName<class AAudioComponent>(std::string& _outName)
    {
        _outName += "-audio";
    }

    template<>
    inline void GenerateCompName<class AParticleComponent>(std::string& _outName)
    {
        _outName += "-particle";
    }

    template<>
    inline void GenerateCompName<class AAnimateComponent>(std::string& _outName)
    {
        _outName += "-animate";
    }

    template<>
    inline void GenerateCompName<class UTransformComponent>(std::string& _outName)
    {
        _outName += "-transform";
    }

    template<>
    inline void GenerateCompName<class USpriteComponent>(std::string& _outName)
    {
        _outName += "-sprite";
    }

    template<>
    inline void GenerateCompName<class UAnimateComponent>(std::string& _outName)
    {
        _outName += "-animate";
    }

    template<>
    inline void GenerateCompName<class UTimerComponent>(std::string& _outName)
    {
        _outName += "-timer";
    }

    template<>
    inline void GenerateCompName<class UInputComponent>(std::string& _outName)
    {
        _outName += "-input";
    }

    template<>
    inline void GenerateCompName<class UInteractComponent>(std::string& _outName)
    {
        _outName += "-interact";
    }

    template<>
    inline void GenerateCompName<class UButtonComponent>(std::string& _outName)
    {
        _outName += "-button";
    }

    template<>
    inline void GenerateCompName<class UAudioComponent>(std::string& _outName)
    {
        _outName += "-audio";
    }
};


