var globalSet;
var globalGet;

func main() {
  var a = "initial";

  func set(value) { a = value; }
  func get() { print a; }

  globalSet = set;
  globalGet = get;
}

main();
globalSet("changed");
globalGet();
globalSet("changed again");
globalGet();

func makeCounter() {
  var count = 2;
  func counter() {
    count = count + 3;
    print(count);
  }
  count = 10;
  return counter;
}

var myCounter = makeCounter();
var anotherCounter = makeCounter();
myCounter();
myCounter();
myCounter();
myCounter();
anotherCounter();


var getter;
var setter;
func getAndSet() {
  var value = 5;

  func set(newValue) {
    value = newValue;
  }

  func get() {
    return value;
  }

  getter = get;
  setter = set;

}

getAndSet();

print(getter());
setter(20);
print(getter());
print(getter());
