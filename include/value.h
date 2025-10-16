#ifndef asas_value_h
#define asas_value_h

#include "common.h"
#include <cstdio>
#include <iostream>
#include <variant>
#include <type_traits>
#include "object.h"

using Value = std::variant<std::monostate, bool, double, AsasObject*>;

inline void printValue(const Value &value) {
  std::visit([](auto &&v) {
    using V = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<V, std::monostate>)
      printf("nil");
    else if constexpr (std::is_same_v<V, bool>)
      printf("%s", v ? "true" : "false");
    else if constexpr (std::is_same_v<V, double>)
      printf("%.2f", v);
    // print asasString
    else if constexpr (std::is_same_v<V, AsasObject*>) {
      AsasString* strObj = dynamic_cast<AsasString*>(v);
      if (strObj != nullptr)
        printf("%s", strObj->getData());
      else
        printf("Object");
    }
    else printf("%g", v);
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

private:
  std::vector<Value> values_;
};

#endif // asas_value_h
