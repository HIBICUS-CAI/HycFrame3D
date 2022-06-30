#pragma once

#include "Hyc3DCommon.h"
#include <string>

class ComponentGetter
{
    friend class ActorObject;
    friend class UiObject;

    template<typename T>
    static void GenerateCompName(std::string& _outName)
    {
        _outName = "this comp type doesn't exist";
    }

    template<>
    static void GenerateCompName<class ATransformComponent>(std::string& _outName)
    {
        _outName += "-transform";
    }

    template<>
    static void GenerateCompName<class AInputComponent>(std::string& _outName)
    {
        _outName += "-input";
    }

    template<>
    static void GenerateCompName<class AInteractComponent>(std::string& _outName)
    {
        _outName += "-interact";
    }

    template<>
    static void GenerateCompName<class ATimerComponent>(std::string& _outName)
    {
        _outName += "-timer";
    }

    template<>
    static void GenerateCompName<class ACollisionComponent>(std::string& _outName)
    {
        _outName += "-collision";
    }

    template<>
    static void GenerateCompName<class ASpriteComponent>(std::string& _outName)
    {
        _outName += "-sprite";
    }

    template<>
    static void GenerateCompName<class AMeshComponent>(std::string& _outName)
    {
        _outName += "-mesh";
    }

    template<>
    static void GenerateCompName<class ALightComponent>(std::string& _outName)
    {
        _outName += "-light";
    }

    template<>
    static void GenerateCompName<class AAudioComponent>(std::string& _outName)
    {
        _outName += "-audio";
    }

    template<>
    static void GenerateCompName<class AParticleComponent>(std::string& _outName)
    {
        _outName += "-particle";
    }

    template<>
    static void GenerateCompName<class AAnimateComponent>(std::string& _outName)
    {
        _outName += "-animate";
    }

    template<>
    static void GenerateCompName<class UTransformComponent>(std::string& _outName)
    {
        _outName += "-transform";
    }

    template<>
    static void GenerateCompName<class USpriteComponent>(std::string& _outName)
    {
        _outName += "-sprite";
    }

    template<>
    static void GenerateCompName<class UAnimateComponent>(std::string& _outName)
    {
        _outName += "-animate";
    }

    template<>
    static void GenerateCompName<class UTimerComponent>(std::string& _outName)
    {
        _outName += "-timer";
    }

    template<>
    static void GenerateCompName<class UInputComponent>(std::string& _outName)
    {
        _outName += "-input";
    }

    template<>
    static void GenerateCompName<class UInteractComponent>(std::string& _outName)
    {
        _outName += "-interact";
    }

    template<>
    static void GenerateCompName<class UButtonComponent>(std::string& _outName)
    {
        _outName += "-button";
    }

    template<>
    static void GenerateCompName<class UAudioComponent>(std::string& _outName)
    {
        _outName += "-audio";
    }
};


