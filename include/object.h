#ifndef asas_object_h
#define asas_object_h

#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include "chunk.h"

class AsasObject {
public:
  virtual ~AsasObject() = default;
};

class AsasString : public AsasObject {
public:
  AsasString(const char *data) {
    refCountObjects_++;

    length_ = strlen(data);
    this->data_ = new char[length_ + 1];
    memcpy(this->data_, data, length_);
    this->data_[length_] = '\0';
  }
  ~AsasString() override { refCountObjects_--; delete[] data_; }
  static int getRefCountObjects()  { return refCountObjects_; }

  int getLength() const { return length_; }
  const char *getData() const { return data_; }
private:
  int length_;
  char *data_;

  inline static int refCountObjects_ = 0;
};

class AsasFunction : public AsasObject {
public:
  AsasFunction(Chunk *chunk, std::string name = "")
      : arity(0), name_(name), chunk_(chunk), upvalueCount_(0)
  {
    refCountObjects_++;
  }
  ~AsasFunction() override { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }

  std::string getName() const { return name_; }
  Chunk *getChunk() const { return chunk_; }
  
  void addInstruction(uint8_t instruction, int line) {
    if (chunk_) chunk_->write(instruction, line);
  }
  int getUpvalueCount() const { return upvalueCount_; }
  void incrementUpvalueCount() { upvalueCount_++; }

  int arity;
private:
  std::string name_;
  Chunk *chunk_;
  int upvalueCount_;

  inline static int refCountObjects_ = 0;
};

class AsasNativeFunction : public AsasObject {
public:
  using NativeFn = std::function<Value(const std::vector<Value>&)>;
  AsasNativeFunction(NativeFn fn, std::string name = "")
      : function_(std::move(fn)), name_(std::move(name))
  {
    refCountObjects_++;
  }
  ~AsasNativeFunction() override { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }
  std::string getName() const { return name_; }
  Value call(const std::vector<Value> &args) const {
    return function_(args);
  }

private:
  NativeFn function_;
  std::string name_;

  inline static int refCountObjects_ = 0;
};

class AsasUpvalue : public AsasObject {
public:
  explicit AsasUpvalue(Value *location)
      : location_(location)
  {
    refCountObjects_++;
  }
  ~AsasUpvalue() override { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }
  Value* getLocation() const { return location_; }
  void close() {
    closedValue_ = *location_;
    location_ = &closedValue_;
  }

private:
  Value *location_;
  Value closedValue_;
  inline static int refCountObjects_ = 0;
};

class AsasClosure : public AsasObject {
public:
  explicit AsasClosure(AsasFunction *function)
      : function_(function)
  {
    refCountObjects_++;
  }
  ~AsasClosure() override { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }

  AsasFunction* getFunction() const { return function_; }
  void addUpvalue(AsasUpvalue* upvalue) {
    upvalues_.push_back(upvalue);
  }
  AsasUpvalue* getUpvalueAt(int index) const {
    return upvalues_[index];
  }

private:
  AsasFunction *function_;
  std::vector<AsasUpvalue*> upvalues_;
  inline static int refCountObjects_ = 0;
};

#endif // asas_object_h
