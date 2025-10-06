#ifndef asas_value_h
#define asas_value_h

#include "common.h"
#include <cstdio>

typedef double Value;

inline void printValue(Value value) {
  printf("%g", value);
}

class DataValue {
public:
  void write(const Value& value) { values_.push_back(value); }
  const Value& getAt(size_t index) const { return values_[index]; }
  size_t size() const { return values_.size(); }

private:
  std::vector<Value> values_;
};

#endif // asas_value_h
