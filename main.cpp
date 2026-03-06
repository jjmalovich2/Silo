#include "silo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

// `printSymbolTable()` is defined in AST.cpp; use that implementation.

const std::string COMMANDS[] = {"-dump", "-d", "-v", "-version"};
bool dumpMode = false;
const std::string VERSION = "Silo v1.2.Pisces~first-v1_c++17";

void compileCommand(const std::string& cmd) {
    auto cc = std::find(std::begin(COMMANDS), std::end(COMMANDS), cmd); // compile command
    if (cc != std::end(COMMANDS)) {
        if (*cc == "-dump" || *cc == "-d") dumpMode = true;
        if (*cc == "-v" || *cc == "-version") std::cout << VERSION << std::endl << std::endl;
    } else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        return;
    }
}

int main(int argc, char* argv[]) {
    clear(); 
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file> or <power_command>" << std::endl;
        return 1;
    }

    // check for power commands first
    if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "-version") {
        std::cout << VERSION << std::endl;
        return 0;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            compileCommand(argv[i]);
        }
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 1. Lexing
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // 2. Parsing
    Parser parser(tokens);
    
    // Wrap the entire script in a main block for parsing
    // We manually simulate a "Block" by adding braces to the input or just parsing repeatedly
    // But our parseBlock expects braces. 
    // Trick: Wrap tokens in braces virtually or just parse logic.
    // Better: Allow parsing a list of statements without braces for the main file.
    
    // Simplest fix for your current Parser setup:
    // We will assume the file content is wrapped in { ... } implicitly or we just loop statements.
    
    try {
        // Parse statement by statement until EOF
        while (true) {
            // Check if we are at EOF
            // (We need to look at the parser's internal position, but it's private.
            //  Instead, we can catch the EOF exception or check the token vector manually before passing)
            // Actually, let's just create a block node manually and fill it.
            
            auto stmt = parser.parseStatement();
            if (!stmt) break; // EOF or end of valid statements
            stmt->execute();
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] Runtime/Parsing Error: " << e.what() << std::endl;
        return 1;
    }
    
    if (dumpMode) printSymbolTable();
    return 0;
}