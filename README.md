# HycFrame3D

Vulkan 1.3 で描画する 3D フレームワークです。Ubuntu（および CMake 経由で Windows）でビルドできます。

## 必要なもの

- Vulkan 1.3 対応 GPU とドライバ
  - 必須機能: dynamic rendering, synchronization2, shaderDemoteToHelperInvocation,
    geometryShader, shaderStorageImageRead/WriteWithoutFormat
- CMake 3.20 以上、C++17 コンパイラ

Ubuntu 24.04 で必要なパッケージ:

```sh
sudo apt install build-essential cmake ninja-build \
  libvulkan-dev mesa-vulkan-drivers vulkan-validationlayers \
  glslang-dev glslang-tools libsdl2-dev libopenal-dev libbullet-dev
```

DirectXMath・VMA・stb_image などのヘッダは `ThirdParty/Include` に同梱しています。

## ビルド

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

実行ファイルと共有ライブラリは `build/bin` に出力されます。
描画システムの静的リソースはビルド時に `01_HycFrame3D/RenderSystem_StaticResources` へコピーされます。

## 実行

アセットは作業ディレクトリからの相対パスで読み込むため、`01_HycFrame3D` で実行します。

```sh
cd 01_HycFrame3D
../build/bin/01_HycFrame3D
```

ロガー（`04_HycLogger`）はゲームと同じフォルダから起動されます。
シェーダーは HLSL のまま起動時に glslang で SPIR-V へコンパイルされます。

### 検証レイヤー

Debug ビルドでは Vulkan 検証レイヤーが既定で有効です（`HYC_VK_VALIDATION=0` で無効）。
Release ビルドでは環境変数 `HYC_VK_VALIDATION=1` で有効になります。

### GPU のない環境での確認

lavapipe（ソフトウェア Vulkan）と Xvfb で動作確認できます。

```sh
sudo apt install xvfb
cd 01_HycFrame3D
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json \
  xvfb-run -s "-screen 0 1280x720x24" ../build/bin/01_HycFrame3D
```

## 構成

| ディレクトリ | 内容 |
|---|---|
| `01_HycFrame3D` | ゲーム本体、描画パイプライン（`01_MiddleTools/BasicRSPipeline`）とアセット |
| `03_RenderSystem_VK` | Vulkan 描画システム（RHI、リソース管理、シェーダーコンパイル） |
| `03_InputDevice` | SDL2 によるキーボード・マウス・ゲームコントローラー入力 |
| `04_WindowManager` | SDL2 によるウィンドウ管理 |
| `04_HycLogger` | TCP 接続のログビューア |
| `05_HycUtilitiy` | ヘッダのみのユーティリティ |
