#include "parser.hpp"

#include <memory>

using namespace std;

NodePtr Parser::Parse() { return parseBlock({TokenKind::End}); }

NodePtr Parser::parseBlock(initializer_list<TokenKind> terminators) {
    auto block = make_unique<Node>(NodeKind::Block);

    consumeNewlines();

    while (!check(TokenKind::End)) {
        bool atTerminator = false;

        for (TokenKind kind : terminators) {
            if (check(kind)) {
                atTerminator = true;
                break;
            }
        }
        if (atTerminator) break;

        NodePtr statement = parseStatement();

        if (statement) block->children.push_back(std::move(statement));

        consumeNewlines();
        check(TokenKind::Semicolon);
        consumeNewlines();
    }

    return block;
}

NodePtr Parser::parseStatement() {}

NodePtr Parser::parseExpression() {}

NodePtr Parser::parseIf() {}

NodePtr Parser::parseNumber(Token token) {
    auto node = make_unique<Node>(NodeKind::NumberLiteral);
    node->str = token.str;
    return node;
}

NodePtr Parser::parseIden(Token token) {
    auto node = make_unique<Node>(NodeKind::Iden);
    node->str = token.str;
    return node;
}

NodePtr Parser::parseParenExpr(Token token) {
    NodePtr innerExpr = parseExpression();

    consume(TokenKind::Rparen);

    return innerExpr;
}

NodePtr Parser::parseUnaryMinus(Token token) {
    auto node = make_unique<Node>(NodeKind::UnaryOp);
    node->opKind = TokenKind::Minus;

    node->children.push_back(parseExpression());
    return node;
}

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

void Parser::consumeNewlines() {
    while (check(TokenKind::Newline)) {
        advance();
    }
}
