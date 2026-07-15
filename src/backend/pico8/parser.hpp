#pragma once

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
    Goto,
    Label,
    ExprStmt,
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
    Node Parse();

   private:
    std::vector<Token> tokens;
    size_t cursor;

    const Token& peek(int n = 0);
    void advance(int n = 1);
    bool check(TokenKind kind);
    bool consume(TokenKind kind);
};
