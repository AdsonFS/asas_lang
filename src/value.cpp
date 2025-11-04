#include "value.h"
#include "object.h"

AsasString* ValueHelper::toStringObj(const Value &value) {
  if (auto objPtr = std::get_if<AsasObject*>(&value)) {
    return dynamic_cast<AsasString*>(*objPtr);
  }
  throw std::runtime_error("Value is not an AsasObject*");
  return nullptr;
}

AsasFunction* ValueHelper::toFunctionObj(const Value &value) {
  auto objPtr = std::get_if<AsasObject*>(&value);
  if (objPtr == nullptr)
    throw std::runtime_error("Value is not an AsasFunction*");
  auto funcPtr = dynamic_cast<AsasFunction*>(*objPtr);
  if (funcPtr == nullptr)
    throw std::runtime_error("Value is not an AsasFunction*");
  return funcPtr;
}

bool ValueHelper::toBool(const Value &value) {
  if (auto boolPtr = std::get_if<bool>(&value)) {
    return *boolPtr;
  }
  throw std::runtime_error("Value is not a bool");
  return false;
}

AsasString* ValueHelper::tryParseToStringObj(const Value &value) {
  if (auto objPtr = std::get_if<AsasObject*>(&value))
    return dynamic_cast<AsasString*>(*objPtr);
  return nullptr;
}

// void printValue(const Value &value) {
//   std::visit([](auto &&v) {
//     using V = std::decay_t<decltype(v)>;
//     if constexpr (std::is_same_v<V, std::monostate>)
//       return void (printf("nil"));
//     if constexpr (std::is_same_v<V, bool>)
//       return void (printf("%s", v ? "true" : "false"));
//     if constexpr (std::is_same_v<V, double>)
//       return void (printf("%.2f", v));
//     if constexpr (std::is_same_v<V, AsasObject*>) {
//       if (dynamic_cast<AsasString*>(v) != nullptr)
//         return void (printf("%s", dynamic_cast<AsasString*>(v)->getData()));
//       if (dynamic_cast<AsasFunction*>(v) != nullptr)
//         return void (printf("<fn %s>", dynamic_cast<AsasFunction*>(v)->getName().c_str()));
//       if (dynamic_cast<AsasNativeFunction*>(v) != nullptr)
//         return void (printf("<native fn %s>", dynamic_cast<AsasNativeFunction*>(v)->getName().c_str()));
//       if (dynamic_cast<AsasClosure*>(v) != nullptr)
//         return void (printf("<closure %s>", dynamic_cast<AsasClosure*>(v)->getFunction()->getName().c_str()));
//     }
//     throw std::runtime_error("Unknown type in Value variant");
//   }, value);
// }


void printValue(const Value &value) {
  std::visit([](auto &&v) {
    using V = std::decay_t<decltype(v)>;

    if constexpr (std::is_same_v<V, std::monostate>)
      printf("nil");
    else if constexpr (std::is_same_v<V, bool>)
      printf("%s", v ? "true" : "false");
    else if constexpr (std::is_same_v<V, double>)
      printf("%.2f", v);
    else if constexpr (std::is_same_v<V, AsasObject*>) {
      if (!v) return void (printf("nil"));

      if (auto str = dynamic_cast<AsasString*>(v))
        printf("%s", str->getData());
      else if (auto func = dynamic_cast<AsasFunction*>(v))
        printf("<fn %s>", func->getName().c_str());
      else if (auto nativeFn = dynamic_cast<AsasNativeFunction*>(v))
        printf("<native fn %s>", nativeFn->getName().c_str());
      else if (auto closure = dynamic_cast<AsasClosure*>(v))
        printf("<closure %s>", closure->getFunction()->getName().c_str());
      else
        printf("<unknown object>");
    }
  }, value);
}

void printValue(const char* left, const Value &value, const char* right) {
  printf("%s", left);
  printValue(value);
  printf("%s", right);
}

DataValue::~DataValue() {
  for (const Value &value : values_) {
    if (auto objPtr = std::get_if<AsasObject*>(&value))
      delete *objPtr;
  }
}
