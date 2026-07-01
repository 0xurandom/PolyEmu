#pragma once
#include <string>

enum TokenKind {
    IDEN,
    PLUS,
    MINUS,
    ASTERISK,
    PERCENT,
    LPAREN,
    RPAREN,
    EQUALS,
    LESS_THAN,
    GREATER_THAN,
};

struct Token {
    TokenKind kind;
    std::string str;
};
