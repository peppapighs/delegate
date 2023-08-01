#pragma once

#include "Closure.h"

template <typename R, typename... Args> class Delegate {
public:
  Delegate() = default;
  template <typename C, typename D>
  Delegate(const D *object, R (C::*function)(Args...) const)
      : closure_(object, function) {}
  template <typename C, typename D>
  Delegate(D *object, R (C::*function)(Args...) const)
      : closure_(object, function) {}
  template <typename C, typename D>
  Delegate(D *object, R (C::*function)(Args...)) : closure_(object, function) {}
  template <typename C>
  Delegate(const C *object, R (C::*function)(Args...) const)
      : closure_(object, function) {}
  template <typename C>
  Delegate(C *object, R (C::*function)(Args...) const)
      : closure_(object, function) {}
  template <typename C>
  Delegate(C *object, R (C::*function)(Args...)) : closure_(object, function) {}
  Delegate(R (*function)(Args...))
      : closure_(function, &Delegate::StaticFunctionInvoker) {}

  template <typename... Args2> R operator()(Args2 &&...args) const {
    return closure_(std::forward<Args2>(args)...);
  }

  operator bool() const { return bool(closure_); }
  bool operator==(const Delegate &) const = default;
  auto operator<=>(const Delegate &) const = default;

private:
  R StaticFunctionInvoker(Args... args) {
    return reinterpret_cast<R (*)(Args...)>(this)(std::forward<Args>(args)...);
  }

  Closure<R, Args...> closure_;
};