#pragma once

#include "UiComponent.h"

#include <RSCommon.h>

#include <vector>

class USpriteComponent : public UiComponent {
private:
  std::string MeshesName;
  std::string OriginTextureName;

  DirectX::XMFLOAT4 OffsetColor;

public:
  USpriteComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~USpriteComponent();

  USpriteComponent &operator=(const USpriteComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    MeshesName = Source.MeshesName;
    OriginTextureName = Source.OriginTextureName;
    OffsetColor = Source.OffsetColor;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  bool createSpriteMesh(class SceneNode *Scene,
                        const DirectX::XMFLOAT4 &OffsetColorSprite,
                        const std::string &TexName);

  const DirectX::XMFLOAT4 &getOffsetColor() const;
  void setOffsetColor(const DirectX::XMFLOAT4 &OffsetColorSprite);

  void resetTexture();

private:
  void syncTransformDataToInstance();
};
