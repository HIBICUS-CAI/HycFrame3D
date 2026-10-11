//---------------------------------------------------------------
// File: RSExtraMacro.h
// Proj: RenderSystem_VK
// Info: 提供RenderSystem_VK内可用的额外宏定义
// Date: 2021.9.14
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#define FAIL_VK_RETURN(RESULT)                                                 \
  {                                                                            \
    if ((RESULT) != VK_SUCCESS) {                                              \
      return false;                                                            \
    }                                                                          \
  }

#define FAIL_NULL_RETURN(PTR)                                                  \
  {                                                                            \
    if (!(PTR)) {                                                              \
      return false;                                                            \
    }                                                                          \
  }

#define SAFE_RELEASE(PTR)                                                      \
  {                                                                            \
    if (PTR) {                                                                 \
      (PTR)->Release();                                                        \
      (PTR) = nullptr;                                                         \
    }                                                                          \
  }
