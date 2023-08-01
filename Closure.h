#pragma once

#if !(defined(__GNUC__) && defined(__x86_64__) && defined(__linux__))
#error "Unsupported platform"
#endif

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>

namespace {
template <typename T, typename U> inline T ImplicitCast(U u) { return u; }

template <typename T, typename U> inline T BitCast(U u) {
  static_assert(sizeof(T) == sizeof(U), "Unexpected size difference");

  return *reinterpret_cast<T *>(&u);
}
} // namespace

template <typename R, typename... Args> class Closure {
public:
  class Class;
  using FunctionPtr = R (*)(Class *, Args...);

  struct MemberFunctionPtr {
    union {
      FunctionPtr function;
      ptrdiff_t offset;
    };
    ptrdiff_t adjust;
  };

  struct VTable {
    FunctionPtr *functions;
  };

  Closure() = default;
  template <typename C, typename D>
  Closure(const D *object, R (C::*function)(Args...) const)
      : Closure(ImplicitCast<const C *>(object), function) {}
  template <typename C, typename D>
  Closure(D *object, R (C::*function)(Args...) const)
      : Closure(ImplicitCast<C *>(object), function) {}
  template <typename C, typename D>
  Closure(D *object, R (C::*function)(Args...))
      : Closure(ImplicitCast<C *>(object), function) {}
  template <typename C>
  Closure(const C *object, R (C::*function)(Args...) const)
      : Closure(const_cast<C *>(object), function) {}
  template <typename C>
  Closure(C *object, R (C::*function)(Args...) const)
      : Closure(object, reinterpret_cast<R (C::*)(Args...)>(function)) {}

  template <typename C> Closure(C *object, R (C::*function)(Args...)) {
    static_assert(sizeof(function) == sizeof(MemberFunctionPtr),
                  "Unexpected member function pointer size");

    auto mfp = BitCast<MemberFunctionPtr>(function);
    object_ = reinterpret_cast<Class *>(reinterpret_cast<uint8_t *>(object) +
                                        mfp.adjust);
    if (mfp.offset & 1) {
      auto vtable = *reinterpret_cast<VTable *>(object_);
      function_ = vtable.functions[(mfp.offset - 1) / sizeof(FunctionPtr)];
    } else {
      function_ = mfp.function;
    }
  }

  template <typename C>
  Closure(R (*function)(Args...), R (C::*invoker)(Args...))
      : Closure(reinterpret_cast<C *>(function), invoker) {
    static_assert(sizeof(function) == sizeof(Class *),
                  "Unexpected code pointer size");

    object_ = reinterpret_cast<Class *>(function);
  }

  template <typename... Args2> R operator()(Args2 &&...args) const {
    return function_(object_, std::forward<Args2>(args)...);
  }

  operator bool() const { return bool(object_) && bool(function_); }
  bool operator==(const Closure &) const = default;
  auto operator<=>(const Closure &) const = default;

private:
  Class *object_;
  FunctionPtr function_;
};
