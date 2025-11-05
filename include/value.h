#ifndef asas_value_h
#define asas_value_h

#include "common.h"
#include "object_fwd.h"
#include <cstdio>
#include <iostream>
#include <variant>
#include <type_traits>
#include <vector>

using Value = std::variant<std::monostate, bool, double, AsasObject*>;

class ValueHelper {
public:
  static AsasString* toStringObj(const Value &value);
  static AsasFunction* toFunctionObj(const Value &value);
  static AsasString* tryParseToStringObj(const Value &value);

  static bool toBool(const Value &value);
};

void printValue(const Value &value);
void printValue(const char* left, const Value &value, const char* right);

class DataValue {
public:
  void write(const Value &value) { values_.push_back(value); }
  const Value &getAt(size_t index) const { return values_[index]; }
  size_t size() const { return values_.size(); }
  std::vector<Value>& getValues() { return values_; }

  ~DataValue();

private:
  std::vector<Value> values_;
};

#endif // asas_value_h
