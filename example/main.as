var globalOne;
var globalTwo;

func main() {
  for (var a = 1; a <= 2; a = a + 1) {
    func closure() {
      print a;
    }
    if (globalOne == nil) {
      globalOne = closure;
    } else {
      globalTwo = closure;
    }
  }
}

main();
globalOne();
globalTwo();
