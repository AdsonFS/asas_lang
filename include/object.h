#ifndef asas_object_h
#define asas_object_h

#include <cstring>
#include <string>

class Chunk;

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
      : arity(0), name_(name), chunk_(chunk)
  {
    refCountObjects_++;
  }
  ~AsasFunction() override { refCountObjects_--; }
  static int getRefCountObjects()  { return refCountObjects_; }

  std::string getName() const { return name_; }
  Chunk *getChunk() const { return chunk_; }

  int arity;
private:
  // const char *name_;
  std::string name_;
  Chunk *chunk_;

  inline static int refCountObjects_ = 0;
};

#endif // asas_object_h
