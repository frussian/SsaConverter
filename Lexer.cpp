//
// Created by zoomo on 19.05.2022.
//

#include "Lexer.h"
#include <regex>
#include <iostream>
#include <string>

void Token::print() {
    printf("tag: %d, (%d, %d)-(%d, %d)\n",
                      tag, frag.ys, frag.xs,
                      frag.ye, frag.xe);
}

std::vector<Token> Lexer::get_tokens(std::string &program) {
    std::vector<Token> tokens;
    std::vector<std::regex> regs;
    try {
        regs.emplace_back("^if");
        regs.emplace_back("^for");
        regs.emplace_back("^int");
        regs.emplace_back("^return");
        regs.emplace_back("^else");
        regs.emplace_back("^break");
        regs.emplace_back("^,");
        regs.emplace_back("^\\{");
        regs.emplace_back("^\\}");
        regs.emplace_back("^\\(");
        regs.emplace_back("^\\)");
        regs.emplace_back("^[a-zA-Z][a-zA-Z0-9]*");
        regs.emplace_back("^[0-9]+");
        regs.emplace_back("^==");
        regs.emplace_back("^=");
        regs.emplace_back("^\\+");
        regs.emplace_back("^-");
        regs.emplace_back("^\\*");
        regs.emplace_back("^/");
        regs.emplace_back("^<");
        regs.emplace_back("^>");
        regs.emplace_back("^!=");
        regs.emplace_back("^(\r?)\n");
        regs.emplace_back("^ |\t");
    }
    catch (const std::regex_error &e) {
        std::cout << e.what() << "\n";
        std::cout << e.code() << "\n";
        std::cout << tokens.size() << "\n";
        return tokens;
    }
    int x = 1;
    int y = 1;
    for (int start = 0; start < program.length();){
        int i = 0;
        for (const auto& regex: regs) {
            std::smatch match;
            std::string substr = program.substr(start);
            std::regex_search(substr, match, regex);
            if (!match.empty()) {
                std::string val = match[0].str();
                size_t inc = val.length();
                start += inc;
//                std::cout << val << "\n";
                int xs = x, ys = y;
                for (int j = 0; j < val.length(); j++) {
                    if (val[j] == '\n') {
                        y++;
                        x = 1;
                    } else {
                        x++;
                    }
                }
                Fragment frag(xs, ys, x, y);
                if (i == IDENT) {
                    tokens.emplace_back(DomainTag(i), val, frag);
                } else if (i == NUMBER) {
                    int num = atoi(val.data());
                    tokens.emplace_back(DomainTag(i), num, frag);
                } else if (i < END_OF_PROGRAM) {
                    tokens.emplace_back(DomainTag(i), frag);
                }

                break;
            }
            i++;
        }
        if (i == regs.size()) {
            printf("error lexer at (%d, %d)", y, x);
            return std::vector<Token>();
        }
    }


//    regs.p
    tokens.emplace_back(END_OF_PROGRAM, Fragment(x, y, x, y));
    return tokens;
}
