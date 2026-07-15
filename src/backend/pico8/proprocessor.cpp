#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"

using namespace std;

string getPreviousExpression(const vector<Token>& tokens, size_t i);
string getNextExpression(const vector<Token>& tokens, size_t i);
TokenKind lookupCompoundOpReplacement(TokenKind kind);
string lookupBitwiseReplacement(TokenKind kind);

string preprocessTokens(vector<Token> tokens) {
    string output;
    unordered_map<size_t, string> addendum;

    return output;
}

string getPreviousExpression(const vector<Token>& tokens, size_t i) {
    // TODO: going back to newline doesnt always work
    size_t initial_i = i;

    while (i > 0 && tokens.at(i).kind != TokenKind::Newline) {
        i--;
    }
    size_t final_i = i;

    if (i == 0) {
        cerr << "Warning: tried getting previous expression at i = 0\n";
        return vector<Token>{};
    }

    vector<Token> expr(tokens.begin() + final_i + 1,
                       tokens.begin() + initial_i - 1);
    return expr;
}

string getNextExpression(const vector<Token>& tokens, size_t i) {
    size_t initial_i = i;

    while (i < tokens.size() && tokens.at(i).kind != TokenKind::Newline) {
        i++;
    }

    size_t final_i = i;

    if (i == 0) {
        cerr << "Warning: tried getting previous expression at i = 0\n";
        return vector<Token>{};
    }

    vector<Token> expr(tokens.begin() + initial_i + 1,
                       tokens.begin() + final_i - 1);
    return expr;
}

TokenKind lookupCompoundOpReplacement(TokenKind kind) {
    switch (kind) {
        case TokenKind::PlusEquals:
            return "+";
        case TokenKind::MinusEquals:
            return "-";
        case TokenKind::AsteriskEquals:
            return "*";
        case TokenKind::SlashEquals:
            return "/";
        case TokenKind::PercentEquals:
            return "%";
        case TokenKind::CaretEquals:
            return "^";
        case TokenKind::DotDotEquals:
            return "..";

        default: {
            cerr << "Error: Unknown compound operator\n";
        }
    }
}

string lookupBitwiseReplacement(TokenKind kind) {
    switch (kind) {
        case TokenKind::IntegerDivide:
            return "__idiv";
        case TokenKind::BitwiseAnd:
            return "__band";
        case TokenKind::BitwiseOr:
            return "__bor";
        case TokenKind::BitwiseXor:
            return "__bxor";
        case TokenKind::ShiftLeft:
            return "__shl";
        case TokenKind::ShiftRight:
            return "__shr";
        case TokenKind::ArithShiftRight:
            return "__ashr";
        case TokenKind::BitwiseNot:
            return "__bnot";

        default: {
            cerr << "Error: Unknown bitwise operator\n";
        }
    }
}
