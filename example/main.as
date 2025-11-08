func fib(n) {
  if (n <= 1)
    return n;
  return fib(n - 1) + fib(n - 2);
}

for (var i = 0; i < 10; i = i + 1) {
  print("fib(" + i + ") = " + fib(i));
}

func makeCounter() {
  var count;
  func counter() {
    count = count + 1;
    return count;
  }

  count = 10;
  return counter;
}

var counter1 = makeCounter();
var counter2 = makeCounter();

print("Counter 1: " + counter1());
print("Counter 1: " + counter1());
print("Counter 2: " + counter2());
print("Counter 1: " + counter1());
print("Counter 2: " + counter2());

var str = "";
while (str != "aaaaaa") {
  str = str + "a";
}
print("Final string: " + str);
