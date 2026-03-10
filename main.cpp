#include "silo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <set>
#include <cstdlib>

const std::string COMMANDS[] = {"-dump", "-d", "-v", "-version"};
bool dumpMode = false;
const std::string VERSION = "Fixed Crits\n\nSilo v1.2.Pisces~Magician_c++17";

void compileCommand(const std::string& cmd) {
    auto cc = std::find(std::begin(COMMANDS), std::end(COMMANDS), cmd);
    if (cc != std::end(COMMANDS)) {
        if (*cc == "-dump" || *cc == "-d") dumpMode = true;
        if (*cc == "-v" || *cc == "-version") std::cout << VERSION << std::endl << std::endl;
    } else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        return;
    }
}

// =====================================================================
// PREPROCESSOR
// =====================================================================

// Returns ~/.silo/lib/
std::string getSiloLibDir() {
    const char* home = getenv("HOME");
    if (!home)
        throw std::runtime_error("Cannot find HOME directory. Is $HOME set?");
    return std::string(home) + "/.silo/lib/";
}

// Recursively preprocesses a .sl file, resolving all #include directives.
// <file>  -> looks in ~/.silo/lib/
// "file"  -> looks relative to the current source file
std::string preprocess(const std::string& filepath, std::set<std::string>& included) {
    if (included.count(filepath)) return ""; // already included, skip
    included.insert(filepath);

    std::ifstream file(filepath);
    if (!file)
        throw std::runtime_error("Cannot open file: " + filepath);

    // Base directory of this file (for resolving local "" includes)
    std::string baseDir = "";
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        baseDir = filepath.substr(0, lastSlash + 1);

    std::string result;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) { result += "\n"; continue; }
        std::string trimmed = line.substr(start);

        if (trimmed.rfind("#include", 0) == 0) {
            size_t i = 8; // skip "#include"
            while (i < trimmed.size() && trimmed[i] == ' ') i++;

            if (i >= trimmed.size())
                throw std::runtime_error("Empty #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath);

            char openChar  = trimmed[i];
            char closeChar = (openChar == '<') ? '>' : '"';

            if (openChar != '<' && openChar != '"')
                throw std::runtime_error("Malformed #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath
                                         + " — expected < or \"");

            size_t nameStart = i + 1;
            size_t nameEnd   = trimmed.find(closeChar, nameStart);
            if (nameEnd == std::string::npos)
                throw std::runtime_error("Unclosed #include on line "
                                         + std::to_string(lineNum)
                                         + " of " + filepath);

            std::string name = trimmed.substr(nameStart, nameEnd - nameStart);

            // Add .sl if no extension provided
            if (name.find('.') == std::string::npos)
                name += ".sl";

            std::string includePath;
            if (openChar == '<') {
                // System library: ~/.silo/lib/
                includePath = getSiloLibDir() + name;
            } else {
                // Local file: relative to the current source file
                includePath = baseDir + name;
            }

            result += preprocess(includePath, included);
            result += "\n";
        } else {
            result += line + "\n";
        }
    }

    return result;
}

// =====================================================================
// MAIN
// =====================================================================

int main(int argc, char* argv[]) {
    clear();
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file> or <power_command>" << std::endl;
        return 1;
    }

    // Check for power commands first
    if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "-version") {
        std::cout << VERSION << std::endl;
        return 0;
    }

    // Check file exists before preprocessing
    {
        std::ifstream check(argv[1]);
        if (!check) {
            std::cerr << "Error: Could not open file " << argv[1] << std::endl;
            return 1;
        }
    }

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            compileCommand(argv[i]);
        }
    }

    // 1. Preprocessing — resolve all #include directives
    std::string source;
    try {
        std::set<std::string> included;
        source = preprocess(std::string(argv[1]), included);
    } catch (const std::exception& e) {
        std::cerr << "[!] Preprocessor Error: " << e.what() << std::endl;
        return 1;
    }

    // 2. Lexing
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // 3. Parsing & execution
    Parser parser(tokens);
    try {
        while (true) {
            auto stmt = parser.parseStatement();
            if (!stmt) break;
            stmt->execute();
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] Runtime/Parsing Error: " << e.what() << std::endl;
        return 1;
    }

    if (dumpMode) printSymbolTable();
    return 0;
}