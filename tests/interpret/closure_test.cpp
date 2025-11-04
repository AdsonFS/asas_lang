#include <gtest/gtest.h>
#include "../asas_fixture.h"

TEST(ClosureTest, SimpleClosure) {
  const char *source =
      "func makeCounter() {\n"
      "  var count = 0;\n"
      "  func counter() {\n"
      "    count = count + 1;\n"
      "    return count;\n"
      "  }\n"
      "  return counter;\n"
      "}\n"
      "var myCounter = makeCounter();\n"
      "print myCounter();\n"
      "print myCounter();\n"
      "print myCounter();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n-> 3.00\n");
}

TEST(ClosureTest, ClosureRetainsStateIndependently) {
  const char *source =
      "func makeCounter() {\n"
      "  var count = 0;\n"
      "  func counter() {\n"
      "    count = count + 1;\n"
      "    return count;\n"
      "  }\n"
      "  return counter;\n"
      "}\n"
      "var myCounter = makeCounter();\n"
      "var anotherCounter = makeCounter();\n"
      "print myCounter();\n"
      "print anotherCounter();\n"
      "print myCounter();\n"
      "print myCounter();\n"
      "print anotherCounter();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 1.00\n-> 2.00\n-> 3.00\n-> 2.00\n");
}

TEST(ClosureTest, ClosureWithParameters) {
  const char *source =
      "func makeMultiplier(factor) {\n"
      "  func multiply(x) {\n"
      "    return x * factor;\n"
      "  }\n"
      "  return multiply;\n"
      "}\n"
      "var double = makeMultiplier(2);\n"
      "var triple = makeMultiplier(3);\n"
      "print double(5);\n"
      "print triple(5);\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 10.00\n-> 15.00\n");
}

TEST(ClosureTest, NestedClosures) {
  const char *source =
      "func outer(x) {\n"
      "  func middle(y) {\n"
      "    func inner(z) {\n"
      "      return x + y + z;\n"
      "    }\n"
      "    return inner;\n"
      "  }\n"
      "  return middle;\n"
      "}\n"
      "var add5 = outer(5);\n"
      "var add5And10 = add5(10);\n"
      "print add5And10(3);\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 18.00\n");
}

TEST(ClosureTest, MultipleClosures) {
  const char *source =
      "func makeAdder(x) {\n"
      "  func adder(y) {\n"
      "    return x + y;\n"
      "  }\n"
      "  return adder;\n"
      "}\n"
      "var add5 = makeAdder(5);\n"
      "var add10 = makeAdder(10);\n"
      "print add5(3);\n"
      "print add10(7);\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 8.00\n-> 17.00\n");
}

TEST(ClosureTest, ClosureModifyingCapturedVariable) {
  const char *source =
      "func makeAccumulator() {\n"
      "  var total = 0;\n"
      "  func accumulate(value) {\n"
      "    total = total + value;\n"
      "    return total;\n"
      "  }\n"
      "  return accumulate;\n"
      "}\n"
      "var acc = makeAccumulator();\n"
      "print acc(5);\n"
      "print acc(10);\n"
      "print acc(3);\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 5.00\n-> 15.00\n-> 18.00\n");
}

TEST(ClosureTest, ClosureWithNoCapturedVariables) {
  const char *source =
      "func greet(name) {\n"
      "  return \"Hello, \" + name + \"!\";\n"
      "}\n"
      "var greeter = greet;\n"
      "print greeter(\"Alice\");\n"
      "print greeter(\"Bob\");\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> Hello, Alice!\n-> Hello, Bob!\n");
}


TEST(ClosureTest, ClosureAssignedToGlobalVariable) {
  const char *source =
      "var globalSet;\n"
      "var globalGet;\n"
      "\n"
      "func main() {\n"
      "  var a = \"initial\";\n"
      "\n"
      "  func set(value) { a = value; }\n"
      "  func get() { print a; }\n"
      "\n"
      "  globalSet = set;\n"
      "  globalGet = get;\n"
      "}\n"
      "\n"
      "main();\n"
      "globalGet();\n"
      "globalSet(\"updated\");\n"
      "globalGet();\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> initial\n-> updated\n");
}

TEST(ClosureTest, ClosureWithLoop) {
  const char *source =
      "func makeCounter() {\n"
      "  var count = 0;\n"
      "  func counter() {\n"
      "    count = count + 1;\n"
      "    return count;\n"
      "  }\n"
      "  return counter;\n"
      "}\n"
      "\n"
      "var myCounter = makeCounter();\n"
      "\n"
      "var i = 0;\n"
      "while (i < 5) {\n"
      "  print myCounter();\n"
      "  i = i + 1;\n"
      "}\n";

  auto [result, output] = AsasFixture::runSourceWithSuccess(source);

  EXPECT_EQ(output, "-> 1.00\n-> 2.00\n-> 3.00\n-> 4.00\n-> 5.00\n");
}
