#define BOOST_TEST_MODULE Delegate_test
#include <boost/test/unit_test.hpp>

#include "Delegate.h"

#include <array>
#include <string>

struct TestFixture {
  static int StaticFunction(int a, int b) { return a + b; }

  class BaseClass {
  public:
    BaseClass(const std::string &name) : name_(name) {}

    std::string Function() { return name_ + "::Function"; }
    std::string FunctionConst() const { return name_ + "::FunctionConst"; }
    virtual std::string FunctionVirtual() {
      return name_ + "::FunctionVirtual";
    }
    static std::string StaticFunction() { return "BaseClass::StaticFunction"; }

  protected:
    std::string name_;
  };

  class OtherClass {
  public:
    virtual void FunctionUnused() {}
    virtual std::string FunctionAbstract() = 0;

  private:
    uint64_t value_;
  };

  class LargeClass {
  private:
    std::array<uint64_t, 1024> array_;
  };

  class DerivedClass : public LargeClass,
                       virtual public BaseClass,
                       virtual public OtherClass {
  public:
    DerivedClass(const std::string &name) : BaseClass(name) {}

    std::string FunctionDerived() { return name_ + "::FunctionDerived"; }
    virtual void FunctionUnused2() {}
    virtual std::string FunctionAbstract() override {
      return name_ + "::FunctionAbstract";
    }

  private:
    std::array<uint64_t, 8> array_;
  };

  class RefClass {
  public:
    int Function(int a) { return a; }
    int FunctionLvalue(int &a) { return a; }
    int FunctionRvalue(int &&a) { return a; }
  };
};

BOOST_FIXTURE_TEST_SUITE(Delegate_test, TestFixture)
BOOST_AUTO_TEST_CASE(Delegate_empty) {
  Delegate<void> delegate;
  BOOST_TEST(!delegate);
}

BOOST_AUTO_TEST_CASE(Delegate_equal) {
  BaseClass base("BaseClass");

  BOOST_TEST(Delegate(&base, &BaseClass::Function) ==
             Delegate(&base, &BaseClass::Function));

  BOOST_TEST(Delegate(&base, &BaseClass::Function) !=
             Delegate(&base, &BaseClass::FunctionConst));
}

BOOST_AUTO_TEST_CASE(Delegate_static) {
  Delegate delegate(&StaticFunction);
  BOOST_TEST(delegate(1, 2) == 3);
}

BOOST_AUTO_TEST_CASE(Delegate_base) {
  BaseClass base("BaseClass");

  BOOST_TEST(Delegate(&base, &BaseClass::Function)() == "BaseClass::Function");

  BOOST_TEST(Delegate(&base, &BaseClass::FunctionConst)() ==
             "BaseClass::FunctionConst");
  BOOST_TEST(Delegate(const_cast<const BaseClass *>(&base),
                      &BaseClass::FunctionConst)() ==
             "BaseClass::FunctionConst");

  BOOST_TEST(Delegate(&base, &BaseClass::FunctionVirtual)() ==
             "BaseClass::FunctionVirtual");

  BOOST_TEST(Delegate(&BaseClass::StaticFunction)() ==
             "BaseClass::StaticFunction");
}

BOOST_AUTO_TEST_CASE(Delegate_derived) {
  DerivedClass derived("DerivedClass");

  BOOST_TEST(Delegate(&derived, &DerivedClass::Function)() ==
             "DerivedClass::Function");

  BOOST_TEST(Delegate(&derived, &BaseClass::FunctionVirtual)() ==
             "DerivedClass::FunctionVirtual");

  BOOST_TEST(Delegate(&derived, &OtherClass::FunctionAbstract)() ==
             "DerivedClass::FunctionAbstract");
  BOOST_TEST(Delegate(&derived, &DerivedClass::FunctionAbstract)() ==
             "DerivedClass::FunctionAbstract");

  BOOST_TEST(Delegate(&derived, &DerivedClass::FunctionDerived)() ==
             "DerivedClass::FunctionDerived");
}

BOOST_AUTO_TEST_CASE(Delegate_ref) {
  RefClass ref;

  int a = 1;
  BOOST_TEST(Delegate(&ref, &RefClass::Function)(1) == 1);
  BOOST_TEST(Delegate(&ref, &RefClass::Function)(a) == 1);

  int b = 2;
  BOOST_TEST(Delegate(&ref, &RefClass::FunctionLvalue)(b) == 2);

  int c = 3;
  BOOST_TEST(Delegate(&ref, &RefClass::FunctionRvalue)(std::move(c)) == 3);
  BOOST_TEST(Delegate(&ref, &RefClass::FunctionRvalue)(3) == 3);
}
BOOST_AUTO_TEST_SUITE_END()
