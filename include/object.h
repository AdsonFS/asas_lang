#ifndef asas_object_h
#define asas_object_h

#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include "chunk.h"

class AsasObject {
public:
  AsasObject() : position_(refTotalObjects_++)
  { 
    refCountObjects_++; 
  }
  AsasObject(const AsasObject&) = delete;
  AsasObject& operator=(const AsasObject&) = delete;
  virtual ~AsasObject() { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }
  static void resetRefCounts() { refCountObjects_ = 0; }
  bool isMarked() const { return isMarked_; }
  void mark() { isMarked_ = true; }
  void unmark() { isMarked_ = false; }
  int getPosition() const { return position_; }

private:
  int position_;
  bool isMarked_ = false;
  inline static int refCountObjects_ = 0;
  inline static int refTotalObjects_ = 0;
};

class AsasString : public AsasObject {
public:
  AsasString(const char *data, int length, bool isInterned = false)
      : isInterned_(isInterned){
    refCountObjects_++;

    length_ = length;
    this->data_ = new char[length_ + 1];
    memcpy(this->data_, data, length_);
    this->data_[length_] = '\0';
#ifdef DEBUG_LOG_GC
    printf("\033[0;32mCreated AsasString: %s: %p\033[0m\n", this->data_, (void*)this);
#endif
  }
  AsasString(const char *data) {
    refCountObjects_++;

    length_ = strlen(data);
    this->data_ = new char[length_ + 1];
    memcpy(this->data_, data, length_);
    this->data_[length_] = '\0';
#ifdef DEBUG_LOG_GC
    printf("\033[0;32mCreated AsasString: %s: %p\033[0m\n", this->data_, (void*)this);
#endif
  }
  ~AsasString() override { 
#ifdef DEBUG_LOG_GC
    printf("\033[0;31mDeleted AsasString: %s: %p\033[0m\n", data_, (void*)this);
#endif
    refCountObjects_--; 
    delete[] data_;
  }
  static int getRefCountObjects()  { return refCountObjects_; }
  static void resetRefCounts() { refCountObjects_ = 0; }

  int getLength() const { return length_; }
  const char *getData() const { return data_; }
  // bool isInterned() const { return isInterned_; }
  bool canDelete() const { return !isInterned_; }
private:
  int length_;
  char *data_;
  bool isInterned_;

  inline static int refCountObjects_ = 0;
};

class AsasFunction : public AsasObject {
public:
  AsasFunction(Chunk *chunk, AsasString *name)
      : arity(0), name_(name), chunk_(chunk), upvalueCount_(0)
  {
    refCountObjects_++;
#ifdef DEBUG_LOG_GC
    printf("\033[0;32mCreated AsasFunction: %s: %p\033[0m\n", name_->getData(), (void*)this);
#endif
  }
  ~AsasFunction() override { 
#ifdef DEBUG_LOG_GC
    printf("\033[0;31mDeleted AsasFunction: %p\033[0m\n", (void*)this);
#endif
    refCountObjects_--; 
    // delete chunk_; 
  }
  static int getRefCountObjects()  { return refCountObjects_; }
  static void resetRefCounts() { refCountObjects_ = 0; }

  std::string getName() const { return name_->getData(); }
  Chunk *getChunk() const { return chunk_; }
  
  void addInstruction(uint8_t instruction, int line) {
    if (chunk_) chunk_->write(instruction, line);
  }
  int getUpvalueCount() const { return upvalueCount_; }
  void incrementUpvalueCount() { upvalueCount_++; }
  AsasString* getAsasStringName() const { return name_; }

  int arity;
private:
  // std::string name_;
  AsasString *name_;
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
  static void resetRefCounts() { refCountObjects_ = 0; }
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
#ifdef DEBUG_LOG_GC
    printf("\033[0;32mCreated AsasUpvalue: %p\033[0m\n", (void*)this);
#endif
  }
  ~AsasUpvalue() override { 
    refCountObjects_--;
#ifdef DEBUG_LOG_GC
    printf("\033[0;31mDeleted AsasUpvalue: %p\033[0m\n", (void*)this);
#endif
  }
  static int getRefCountObjects()  { return refCountObjects_; }
  static void resetRefCounts() { refCountObjects_ = 0; }
  Value* getLocation() const { return location_; }
  void setLocation(Value location) { *location_ = location; }
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
#ifdef DEBUG_LOG_GC
    printf("\033[0;32mCreated AsasClosure: %p\033[0m\n", (void*)this);
    printf("\033[0;32m  with function: %s: %p\033[0m\n", 
           function_->getName().c_str(), (void*)function_);
#endif
  }
  ~AsasClosure() override { 
    refCountObjects_--;
    for (AsasUpvalue* upvalue : upvalues_) {
      // delete upvalue;
    }
#ifdef DEBUG_LOG_GC
    printf("\033[0;31mDeleted AsasClosure: %p\033[0m\n", (void*)this);
#endif
  }
  static int getRefCountObjects()  { return refCountObjects_; }
  static void resetRefCounts() { refCountObjects_ = 0; }

  AsasFunction* getFunction() const { return function_; }
  void addUpvalue(AsasUpvalue* upvalue) {
    upvalues_.push_back(upvalue);
  }
  void setUpValueAt(int index, Value value) {
    upvalues_[index]->setLocation(value);
  }

  AsasUpvalue* getUpvalueAt(int index) const {
    return upvalues_[index];
  }
  const std::vector<AsasUpvalue*>& getUpvalues() const { return upvalues_; }

private:
  AsasFunction *function_;
  std::vector<AsasUpvalue*> upvalues_;
  inline static int refCountObjects_ = 0;
};

// class AsasWrapper {
// public:
//   explicit AsasWrapper(AsasObject* object) : object_(object) {}
//   ~AsasWrapper() {
//     if (object_ == nullptr) return;
//     delete object_;
//     object_ = nullptr;
//   }
//   AsasObject* getObject() const { return object_; }
//   // AsasObject* operator->() const { return object_; }
//   AsasString* toStringObj() const { return dynamic_cast<AsasString*>(object_); }
//
// private:
//   AsasObject* object_;
// };

#endif // asas_object_h
