#include "value.h"
#include "vm.h"

void VM::collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("Garbage collection triggered!\n");
#endif

  markRoots();

  freeObjects();
#ifdef DEBUG_LOG_GC
  printf("Garbage collection completed.\n");
#endif
}

void VM::markRoots() {
  for (Value &value : stack_)
    markValue(&value);
  for (auto &[name, value] : globals_)
    markValue(&value);
  for (CallFrame &frame : callFrames_) {
    AsasClosure* closure = frame.getClosure();
    markValue(reinterpret_cast<Value*>(&closure));
    for (AsasUpvalue* upvalue : closure->getUpvalues()) {
      markValue(reinterpret_cast<Value*>(&upvalue));
    }
    markValue(reinterpret_cast<Value*>(closure->getFunction()));
  }
}

void VM::markValue(Value *value) {
  auto object = std::get_if<AsasObject*>(value);
  if (object == nullptr || (*object)->isMarked()) return;

#ifdef DEBUG_LOG_GC
  printf("Marking object %p of type %s\n", (void*)*object, typeid(**object).name());
  printValue("Value to mark: ", *value, "\n");
#endif

  (*object)->mark();
  std::visit([this](auto &&v) {
    using V = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<V, AsasObject*>) {
      if (v == nullptr) return;

      if (auto str = dynamic_cast<AsasString*>(v))
        return void (markValue(reinterpret_cast<Value*>(&str)));
      if (auto func = dynamic_cast<AsasFunction*>(v)) {
        for (const Value &constant : func->getChunk()->getConstants())
          markValue(const_cast<Value*>(&constant));
        return;
      }
      if (auto upvalue = dynamic_cast<AsasUpvalue*>(v))
        return (markValue(upvalue->getLocation()));
      if (auto closure = dynamic_cast<AsasClosure*>(v)) {
        markValue(reinterpret_cast<Value*>(closure->getFunction()));
        for (AsasUpvalue* upvalue : closure->getUpvalues())
          markValue(reinterpret_cast<Value*>(&upvalue));
        return;
      }
    }
  }, *value);
}

void VM::freeObjects() {
  for (auto it = allocatedObjects_.begin(); it != allocatedObjects_.end(); ) {
    AsasObject* obj = *it;
    if (obj->isMarked()) { obj->unmark(); ++it; } 
    else {
#ifdef DEBUG_LOG_GC
      printf("Freeing object %p of type %s\n", (void*)obj, typeid(*obj).name());
      if (auto str = dynamic_cast<AsasClosure*>(obj)) {
        printf("Closure function: %s\n", str->getFunction()->getName().c_str());
      } 
#endif
      delete obj; 
      it = allocatedObjects_.erase(it); 
    }
  }
}
