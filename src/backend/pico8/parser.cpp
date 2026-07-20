#include "parser.hpp"

#include <iostream>
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

NodePtr Parser::parseStatement() {
    Token token = tokens.at(cursor);

    if (check(TokenKind::KwIf)) return parseIf();
}

NodePtr Parser::parseExpression(Token token) {
    switch (token.kind) {
        case TokenKind::BinNum:
        case TokenKind::DecNum:
        case TokenKind::HexNum:
        case TokenKind::SciNum:
            return parseNumber(token);

        case TokenKind::Iden:
            return parseIden(token);

        case TokenKind::Minus:
            return parseUnaryMinus(token);

        case TokenKind::Lparen:
            return parseParenExpr(token);

        default: {
            cerr << "Error: Unexpected expression token\n";
        }
    }
}

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
    NodePtr innerExpr = parseStatement();

    consume(TokenKind::Rparen);

    return innerExpr;
}

NodePtr Parser::parseUnaryMinus(Token token) {
    auto node = make_unique<Node>(NodeKind::UnaryOp);
    node->opKind = TokenKind::Minus;

    node->children.push_back(parseStatement());
    return node;
}

NodePtr Parser::parseBinaryOp(NodePtr node, Token op) {
    auto newNode = make_unique<Node>(NodeKind::BinaryOp);
    newNode->opKind = op.kind;

    newNode->children.push_back(std::move(node));
}

Precedence Parser::getPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Equals:
            return Precedence::Assignment;

        case TokenKind::EqualsEquals:
        case TokenKind::NotEquals:
            return Precedence::Equality;

        case TokenKind::Less:
        case TokenKind::Greater:
        case TokenKind::LessEquals:
        case TokenKind::GreaterEquals:
            return Precedence::Comparison;
    }
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
