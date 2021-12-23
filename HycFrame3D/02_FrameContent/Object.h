#pragma once

#include "Hyc3DCommon.h"
#include <string>

class Object
{
public:
    Object(std::string&& _objName, class SceneNode& _sceneNode);
    Object(std::string& _objName, class SceneNode& _sceneNode);
    virtual ~Object();

    const std::string& GetObjectName() const;

    STATUS GetObjectStatus() const;
    void SetObjectStatus(STATUS _objStatus);

    class SceneNode& GetSceneNode() const;

public:
    virtual bool Init() = 0;
    virtual void Destory() = 0;

private:
    const std::string mObjectName;
    STATUS mObjectStatus;
    class SceneNode& mSceneNodeOwner;
};
