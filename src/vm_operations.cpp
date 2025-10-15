#include "vm.h"

void VM::opNegate() {
  Value a = pop();
  Value result = std::visit(
      [this](auto &&v) -> Value {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, double>) return -v;
        if constexpr (std::is_same_v<V, bool>) return !v;
        return runtimeError("Operand must be a number or a boolean.");
      }, a);
  push(result);
}

void VM::opNot() {
  Value a = pop();
  push(std::visit(
    [this](auto &&v) -> Value {
      using V = std::decay_t<decltype(v)>;
      if constexpr (std::is_same_v<V, bool>) return v;
      if constexpr (std::is_same_v<V, double>) return v != 0;
      return runtimeError("Operand must be a number or a boolean.");
    }, a));
}

void VM::opAdd() {
  Value b = pop();
  Value a = pop();
  push(std::visit(
      [this](auto &&x, auto &&y) -> Value {
        using X = std::decay_t<decltype(x)>;
        using Y = std::decay_t<decltype(y)>;
        if constexpr (std::is_same_v<X, double> && std::is_same_v<Y, double>)
          return x + y;
        if constexpr (std::is_same_v<X, bool> && std::is_same_v<Y, bool>)
          return x || y;
        return runtimeError("Operands must be two numbers or two booleans.");
      }, a, b));
}

void VM::opSubtract() {
  Value b = pop();
  Value a = pop();
  push(std::visit(
      [this](auto &&x, auto &&y) -> Value {
        using X = std::decay_t<decltype(x)>;
        using Y = std::decay_t<decltype(y)>;
        if constexpr (std::is_same_v<X, double> && std::is_same_v<Y, double>)
          return x - y;
        return runtimeError("Operands must be two numbers.");
      }, a, b));
}

void VM::opMultiply() {
  Value b = pop();
  Value a = pop();
  push(std::visit(
      [this](auto &&x, auto &&y) -> Value {
        using X = std::decay_t<decltype(x)>;
        using Y = std::decay_t<decltype(y)>;
        if constexpr (std::is_same_v<X, double> && std::is_same_v<Y, double>)
          return x * y;
        return runtimeError("Operands must be two numbers.");
      }, a, b));
}

void VM::opDivide() {
  Value b = pop();
  Value a = pop();
  push(std::visit(
      [this](auto &&x, auto &&y) -> Value {
        using X = std::decay_t<decltype(x)>;
        using Y = std::decay_t<decltype(y)>;
        if constexpr (std::is_same_v<X, double> && std::is_same_v<Y, double>)
          return (y == 0) ? runtimeError("Division by zero.") : x / y;
        return runtimeError("Operands must be two numbers.");
      }, a, b));
}
