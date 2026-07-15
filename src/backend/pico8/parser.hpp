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
}
