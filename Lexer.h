//
// Created by zoomo on 19.05.2022.
//

#ifndef LAB3_LEXER_H
#define LAB3_LEXER_H

#include <utility>
#include <vector>
#include <string>

enum DomainTag {
    KEYWORD_IF = 0,
    KEYWORD_FOR,
    KEYWORD_INT,
    KEYWORD_RETURN,
    KEYWORD_ELSE,
    KEYWORD_BREAK,
    COMMA,
    L_CURLY_BR,
    R_CURLY_BR,
    L_ROUND_BR,
    R_ROUND_BR,
    IDENT,
    NUMBER,
    EQ,  //==
    ASG,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LT,  //<
    GT,  //>
    //==
    NE,  //!=
    NL,  //newline
    END_OF_PROGRAM
};

class Fragment {
public:
    int xs, ys, xe, ye;
    Fragment(int xs, int ys, int xe, int ye):
    xs(xs), ys(ys), xe(xe), ye(ye){};
    explicit Fragment() {
        xs = ys = xe = ye = 0;
    }
};

class Token {
public:
    DomainTag tag;
    int num;
    std::string ident;
    Fragment frag;

    Token(DomainTag tag, int num, Fragment frag):
        tag(tag), num(num), frag(frag){};
    Token(DomainTag tag, std::string ident, Fragment frag):
        tag(tag), ident(std::move(ident)), frag(frag){};
    explicit Token(DomainTag tag, Fragment frag):
    tag(tag), frag(frag){};
    ~Token()= default;
    void print();
};

class Lexer {
public:
    static std::vector<Token> get_tokens(std::string &program);
};


#endif //LAB3_LEXER_H
