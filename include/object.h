#ifndef asas_object_h
#define asas_object_h

#include <cstdio>
#include <cstring>
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


#endif // asas_object_h
