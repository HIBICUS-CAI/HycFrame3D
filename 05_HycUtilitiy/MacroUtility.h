#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#endif // __clang__
#pragma once
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

#define DEFINE_SINGLETON(_TYPE)                                                \
private:                                                                       \
  using self_type = _TYPE;                                                     \
  using unique_ptr = std::unique_ptr<self_type>;                               \
                                                                               \
public:                                                                        \
  using singleton = const unique_ptr &;                                        \
  static singleton Instance() { return Ref(); }                                \
  static void Create() {                                                       \
    if (!Ref())                                                                \
      Ref() = MakeUnique();                                                    \
  }                                                                            \
  static void Destroy() { Ref().reset(); }                                     \
                                                                               \
private:                                                                       \
  template <typename... Args>                                                  \
  static unique_ptr MakeUnique(Args &&..._args) {                              \
    struct temp : self_type {                                                  \
      temp() : self_type() {}                                                  \
    };                                                                         \
    return std::move(unique_ptr(new temp(std::forward<Args>(_args)...)));      \
  }                                                                            \
  static unique_ptr &Ref() {                                                   \
    static unique_ptr p = MakeUnique();                                        \
    return p;                                                                  \
  }
