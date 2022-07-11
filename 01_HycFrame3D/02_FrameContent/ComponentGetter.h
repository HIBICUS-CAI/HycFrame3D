#pragma once

#include "Hyc3DCommon.h"

#include <string>

namespace component_name {

template <typename T>
inline void generateCompName(std::string &OutName) {
  OutName = "this comp type doesn't exist";
}

template <>
inline void generateCompName<class ATransformComponent>(std::string &OutName) {
  OutName += "-transform";
}

template <>
inline void generateCompName<class AInputComponent>(std::string &OutName) {
  OutName += "-input";
}

template <>
inline void generateCompName<class AInteractComponent>(std::string &OutName) {
  OutName += "-interact";
}

template <>
inline void generateCompName<class ATimerComponent>(std::string &OutName) {
  OutName += "-timer";
}

template <>
inline void generateCompName<class ACollisionComponent>(std::string &OutName) {
  OutName += "-collision";
}

template <>
inline void generateCompName<class ASpriteComponent>(std::string &OutName) {
  OutName += "-sprite";
}

template <>
inline void generateCompName<class AMeshComponent>(std::string &OutName) {
  OutName += "-mesh";
}

template <>
inline void generateCompName<class ALightComponent>(std::string &OutName) {
  OutName += "-light";
}

template <>
inline void generateCompName<class AAudioComponent>(std::string &OutName) {
  OutName += "-audio";
}

template <>
inline void generateCompName<class AParticleComponent>(std::string &OutName) {
  OutName += "-particle";
}

template <>
inline void generateCompName<class AAnimateComponent>(std::string &OutName) {
  OutName += "-animate";
}

template <>
inline void generateCompName<class UTransformComponent>(std::string &OutName) {
  OutName += "-transform";
}

template <>
inline void generateCompName<class USpriteComponent>(std::string &OutName) {
  OutName += "-sprite";
}

template <>
inline void generateCompName<class UAnimateComponent>(std::string &OutName) {
  OutName += "-animate";
}

template <>
inline void generateCompName<class UTimerComponent>(std::string &OutName) {
  OutName += "-timer";
}

template <>
inline void generateCompName<class UInputComponent>(std::string &OutName) {
  OutName += "-input";
}

template <>
inline void generateCompName<class UInteractComponent>(std::string &OutName) {
  OutName += "-interact";
}

template <>
inline void generateCompName<class UButtonComponent>(std::string &OutName) {
  OutName += "-button";
}

template <>
inline void generateCompName<class UAudioComponent>(std::string &OutName) {
  OutName += "-audio";
}

}; // namespace component_name
