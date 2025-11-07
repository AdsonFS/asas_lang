
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
