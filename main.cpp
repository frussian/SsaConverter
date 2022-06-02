#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <memory>

#include "Lexer.h"
#include "Parser.h"
#include "Semantics.h"
#include "Codegen.h"
#include "CFG.h"

void test() {
    std::string s = "test2 aw     \n";
    std::regex r("^test");
    std::smatch matches;

    std::regex_search(s, matches, r);
    matches.position();
    for (size_t i = 0; i < matches.size(); ++i) {
        std::cout << i << ": '" << matches[i].str() << "'\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: lab4 <file>");
        exit(1);
    }

    std::stringstream program;
    std::ifstream inFile;
    inFile.open(
            argv[1]
            );
    if (inFile.is_open()) {
        program << inFile.rdbuf();
        inFile.close();
    } else {
        printf("cannot open file %s", argv[1]);
        exit(0);
    }

    std::string prog_str = program.str();
    auto tokens = Lexer::get_tokens(prog_str);
//    for (auto & token : tokens) {
//        token.print();
//    }
    if (tokens.empty()) {
        return -1;
    }
    Parser pars(tokens);
    auto prog = pars.parse();
    if (!prog) {
        printf("parser failed\n");
        return -2;
    }
    bool ok = Semantics::analyse(prog.get());
    if (!ok) {
        printf("semantics failed\n");
        return -3;
    }

//    CodeGen::genCode(prog.get());
    CFG cfg(prog.get());
    cfg.genCFG();
    cfg.toSsa();
    cfg.genGraphiz("graph.out");

    return 0;
}
