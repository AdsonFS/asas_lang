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
    markValue(&value, false);
  for (auto &[name, value] : globals_)
    markValue(&value, false);
  // for (auto &[name, upvalue] : openUpvalues_)
    // markObject(upvalue);
  for (CallFrame &frame : callFrames_) {
    AsasClosure* closure = frame.getClosure();
    markObject(closure, false);
    // markValue(reinterpret_cast<Value*>(&closure));
    for (AsasUpvalue* upvalue : closure->getUpvalues()) {
      // markValue(reinterpret_cast<Value*>(&upvalue));
      markObject(upvalue, false);
    }
    // markValue(reinterpret_cast<Value*>(closure->getFunction()));
    markObject(closure->getFunction(), false);
  }
}

void VM::setupGarbageCollector(AsasObject* rootScript) {
  markObject(rootScript, true);
}

void VM::markValue(Value *value, bool traceObject) {
  std::visit([this, traceObject](auto &&v) {
    using V = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<V, AsasObject*>)
      markObject(v, traceObject);
  }, *value);
}

void VM::markObject(AsasObject *object, bool traceObject) {
  if (object == nullptr || object->isMarked()) return;
  object->mark();
  if (traceObject) allocatedObjects_.insert(object);

#ifdef DEBUG_LOG_GC
  printf("\033[0;35m");
  printf("Marking object %p of type %s\n", (void*)object, typeid(*object).name());
  printf("\033[0m");
  // printValue("Value to mark: ", *reinterpret_cast<Value*>(object), "\n");
#endif

  if (auto fn = dynamic_cast<AsasFunction*>(object)) {
    // markValue(reinterpret_cast<Value*>(fn->getAsasStringName()));
    markObject(fn->getAsasStringName(), traceObject);
    for (const Value &constant : fn->getChunk()->getConstants())
      markValue(const_cast<Value*>(&constant), traceObject);
      // markValue(const_cast<Value*>(&constant));
      // markValue(const_cast<Value*>(&constant));
    return;
  }
  if (auto upvalue = dynamic_cast<AsasUpvalue*>(object)) {
    markValue(upvalue->getLocation(), traceObject);
    return;
  }
  if (auto closure = dynamic_cast<AsasClosure*>(object)) {
    markObject(closure->getFunction(), traceObject);
    for (AsasUpvalue* upvalue : closure->getUpvalues())
      markObject(upvalue, traceObject);
    return;
  }
}


void VM::freeObjects() {
  for (auto it = allocatedObjects_.begin(); it != allocatedObjects_.end(); ) {
    AsasObject* obj = *it;
    if (obj->isMarked()) { obj->unmark(); ++it; } 
    else {
#ifdef DEBUG_LOG_GC
      printf("Freeing object %p of type %s\n", (void*)obj, typeid(*obj).name());
#endif
      delete obj; 
      it = allocatedObjects_.erase(it); 
      obj = nullptr;
    }
  }
}
