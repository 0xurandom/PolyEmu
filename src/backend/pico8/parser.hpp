#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "lexer.hpp"

enum class NodeKind {
    NumberLiteral,
    StringLiteral,
    BoolLiteral,
    NilLiteral,
    VarArg,
    Iden,

    BinaryOp,
    UnaryOp,
    Call,
    Index,
    FieldAccess,
    TableConstructor,
    FunctionExpr,

    Block,
    Assign,
    LocalAssign,
    CompoundAssign,
    If,
    While,
    NumericFor,
    GenericFor,
    RepeatUntil,
    FunctionDecl,
    Return,
    Break,
    Label,
    ExprStmt,
};

enum class Precedence {
    None,
    Assignment,

    LogicalOr,
    LogicalAnd,
    Equality,
    Comparison,

    BitwiseOr,
    BitwiseXor,
    BitwiseAnd,
    BitwiseShift,

    Concat,
    Term,
    Factor,
    Unary,
    Exponent,
    Call,
    Primary,
};

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;

    std::string str;
    TokenKind opKind;

    std::vector<NodePtr> children;

    Node(NodeKind k) : kind(k) {}
};

class Parser {
   public:
    NodePtr Parse();

   private:
    std::vector<Token> tokens;
    size_t cursor;

    NodePtr parseBlock(std::initializer_list<TokenKind> terminators);
    NodePtr parseStatement();
    NodePtr parseExpression();

    NodePtr parseIf();

    NodePtr parseNumber(Token token);
    NodePtr parseIden(Token token);
    NodePtr parseParenExpr(Token token);
    NodePtr parseUnaryMinus(Token token);

    const Token& peek(int n = 0);
    void advance(int n = 1);
    bool check(TokenKind kind);
    bool consume(TokenKind kind);

    void consumeNewlines();
};
