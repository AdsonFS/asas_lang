#ifndef asas_object_h
#define asas_object_h

#include <cstring>
class AsasObject {
public:
  virtual ~AsasObject() = default;
};
class AsasString : public AsasObject {
public:
  AsasString(const char *data) {
    length = strlen(data);
    this->data = new char[length + 1];
    memcpy(this->data, data, length);
    this->data[length] = '\0';
  }
  ~AsasString() override { delete[] data; }

  int getLength() const { return length; }
  const char *getData() const { return data; }
private:
  int length;
  char *data;
};

#endif // asas_object_h
