#include "silo.h"
#include <iostream>
#include <fstream>
#include <sstream>

// `printSymbolTable()` is defined in AST.cpp; use that implementation.

int main(int argc, char* argv[]) {
    clear(); 
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
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
    
    printSymbolTable();
    return 0;
}