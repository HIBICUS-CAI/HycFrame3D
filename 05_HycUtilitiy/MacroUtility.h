#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <memory>

#define DEFINE_SINGLETON(_TYPE)                                                \
private:                                                                       \
  using SelfType = _TYPE;                                                      \
  using UniquePtr = std::unique_ptr<SelfType>;                                 \
                                                                               \
public:                                                                        \
  using Singleton = const UniquePtr &;                                         \
  static Singleton instance() { return ref(); }                                \
  static void create() {                                                       \
    if (!ref())                                                                \
      ref() = makeUnique();                                                    \
  }                                                                            \
  static void terminate() { ref().reset(); }                                   \
                                                                               \
private:                                                                       \
  template <typename... Args>                                                  \
  static UniquePtr makeUnique(Args &&..._args) {                               \
    struct Temp : SelfType {                                                   \
      Temp() : SelfType() {}                                                   \
    };                                                                         \
    auto TempPtr = UniquePtr(new Temp(std::forward<Args>(_args)...));          \
    return std::move(TempPtr);                                                 \
  }                                                                            \
  static UniquePtr &ref() {                                                    \
    static UniquePtr p = makeUnique();                                         \
    return p;                                                                  \
  }
