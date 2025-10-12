#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"

static void repl() {
  std::string line;
  for (;;) {
    std::cout << "> " << std::flush;

    if (!std::getline(std::cin, line)) {
      std::cout << "\n";
      break;
    }

    // interpret(line.c_str());
  }
}

// leitura de arquivo inteiro em string
static std::string readFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "Could not open file \"" << path << "\".\n";
    std::exit(74);
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static void runFile(const std::string &path) {
  std::cout << "Running file: " << path << std::endl;
  std::string source = readFile(path);


  // Compiler::compile(source.c_str());
  VM vm;
  InterpretResult result = vm.interpret(source.c_str());

  if (result == INTERPRET_COMPILE_ERROR) std::exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) std::exit(70);
}

int main(int argc, const char *argv[]) {
  runFile("example/main.as");
  if (argc == 1)
    repl();
  else if (argc == 2)
    runFile(argv[1]);
  else {
    fprintf(stderr, "Usage: asas [path]\n");
    exit(64);
  }

  // runFile("../example/main.txt");
  // if(argc == 1) repl();
  // else if(argc == 2) runFile(argv[1]);
  // else return 64;

  /*
    Chunk chunk;
    VM vm(chunk);

    int constantIndex = chunk.addConstant(1.2);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(constantIndex, 123);

    constantIndex = chunk.addConstant(3.4);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(constantIndex, 123);

    chunk.write(OP_ADD, 123);

    constantIndex = chunk.addConstant(5.6);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(constantIndex, 123);

    chunk.write(OP_DIVIDE, 123);
    chunk.write(OP_NEGATE, 123);
    chunk.write(OP_RETURN, 124);

    vm.interpret();
    // DebugChunk::disassembleChunk(chunk, "test chunk");
  */

  return 0;
}
