#ifndef asas_value_h
#define asas_value_h

#include "common.h"
#include <cstdio>
#include <iostream>
#include <variant>
#include <type_traits>
#include "object.h"

using Value = std::variant<std::monostate, bool, double, AsasObject*>;

class ValueHelper {
public:
  static AsasString* toStringObj(const Value &value) {
    if (auto objPtr = std::get_if<AsasObject*>(&value)) {
      return dynamic_cast<AsasString*>(*objPtr);
    }
    throw std::runtime_error("Value is not an AsasObject*");
    return nullptr;
  }

  static bool toBool(const Value &value) {
    if (auto boolPtr = std::get_if<bool>(&value)) {
      return *boolPtr;
    }
    throw std::runtime_error("Value is not a bool");
    return false;
  }

  static AsasString* tryParseToStringObj(const Value &value) {
    if (auto objPtr = std::get_if<AsasObject*>(&value))
      return dynamic_cast<AsasString*>(*objPtr);
    return nullptr;
  }

};

inline void printValue(const Value &value) {
  std::visit([](auto &&v) {
    using V = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<V, std::monostate>)
      return void (printf("nil"));
    if constexpr (std::is_same_v<V, bool>)
      return void (printf("%s", v ? "true" : "false"));
    if constexpr (std::is_same_v<V, double>)
      return void (printf("%.2f", v));
    if constexpr (std::is_same_v<V, AsasObject*>) {
      if (dynamic_cast<AsasString*>(v) != nullptr)
        return void (printf("%s", dynamic_cast<AsasString*>(v)->getData()));
      if (dynamic_cast<AsasFunction*>(v) != nullptr)
        return void (printf("<fn %s>", dynamic_cast<AsasFunction*>(v)->getName().c_str()));
      else
        return void (printf("Object"));
    }
    throw std::runtime_error("Unknown type in Value variant");
  }, value);
}

inline void printValue(const char* left, const Value &value, const char* right) {
  printf("%s", left);
  printValue(value);
  printf("%s", right);
}

class DataValue {
public:
  void write(const Value &value) { values_.push_back(value); }
  const Value &getAt(size_t index) const { return values_[index]; }
  size_t size() const { return values_.size(); }

  ~DataValue() {
    for (const Value &value : values_) {
      if (auto objPtr = std::get_if<AsasObject*>(&value))
        delete *objPtr;
    }
  }

private:
  std::vector<Value> values_;
};

#endif // asas_value_h
