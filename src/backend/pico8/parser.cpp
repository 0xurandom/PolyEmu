#include "parser.hpp"

const Token& Parser::peek(int n) { return tokens.at(cursor + n); }

void Parser::advance(int n) { cursor = cursor + n; }

bool Parser::check(TokenKind kind) { return (tokens.at(cursor).kind == kind); }

bool Parser::consume(TokenKind kind) {
    if (tokens.at(cursor).kind == kind) {
        cursor++;
        return true;

    } else {
        return false;
    }
}
