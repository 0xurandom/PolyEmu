#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "lexer.hpp"

using namespace std;

vector<Token> getPreviousExpression(vector<Token>& tokens, size_t i);
vector<Token> getNextExpression(vector<Token>& tokens, size_t i);
TokenKind lookupCompoundOpReplacement(TokenKind kind);
string lookupBitwiseReplacement(TokenKind kind);

string generateLua(vector<Token> tokens, Lexer lexer) {
    string output = "";

    for (const auto& token : tokens) {
        switch (token.kind) {
            case TokenKind::Iden: {
                output += token.str;
                break;
            }

            case TokenKind::DecNum:
            case TokenKind::HexNum:
            case TokenKind::BinNum:
            case TokenKind::SciNum: {
                output += token.str;
                break;
            }

            case TokenKind::Str: {
                output += '\"' + token.str + '\"';
                break;
            }

            case TokenKind::Plus: {
                output += '+';
                break;
            }

            case TokenKind::Minus: {
                output += '-';
                break;
            }

            case TokenKind::Asterisk: {
                output += '*';
                break;
            }

            case TokenKind::Slash: {
                output += '\\';
                break;
            }

            case TokenKind::Percent: {
                output += '%';
                break;
            }

            case TokenKind::Lparen: {
                output += '(';
                break;
            }

            case TokenKind::Rparen: {
                output += ')';
                break;
            }

            case TokenKind::Lbrace: {
                output += '{';
                break;
            }

            case TokenKind::Rbrace: {
                output += '}';
                break;
            }

            case TokenKind::Lbracket: {
                output += '[';
                break;
            }

            case TokenKind::Rbracket: {
                output += ']';
                break;
            }

            case TokenKind::Semicolon: {
                output += ';';
                break;
            }

            case TokenKind::Colon: {
                output += ':';
                break;
            }

            case TokenKind::Comma: {
                output += ',';
                break;
            }

            case TokenKind::Dot: {
                output += '.';
                break;
            }

            case TokenKind::Equals: {
                output += '=';
                break;
            }

            case TokenKind::Caret: {
                output += '^';
                break;
            }

            case TokenKind::Hash: {
                output += '#';
                break;
            }

            case TokenKind::Less: {
                output += '<';
                break;
            }

            case TokenKind::Greater: {
                output += '>';
                break;
            }

                // TODO: check if plus equals are allowed

            case TokenKind::NotEquals: {
                output += "~=";
                break;
            }

                // TODO: check integerdivide

            case TokenKind::BitwiseAnd: {
                output += '&';
                break;
            }

            case TokenKind::BitwiseOr: {
                output += '|';
                break;
            }

            case TokenKind::BitwiseXor: {
                output += "^^";
                break;
            }

            case TokenKind::BitwiseNot: {
                output += '~';
                break;
            }

            case TokenKind::ShiftLeft: {
                output += "<<";
                break;
            }

            case TokenKind::ShiftRight: {
                output += ">>";
                break;
            }

                // TODO: check arith shift right

            case TokenKind::Flr: {
                output += "flr";
                break;
            }

            case TokenKind::Ceil: {
                output += "ceil";
                break;
            }

            case TokenKind::KwBreak:
            case TokenKind::KwDo:
            case TokenKind::KwElse:
            case TokenKind::KwElseif:
            case TokenKind::KwEnd:
            case TokenKind::KwFalse:
            case TokenKind::KwFor:
            case TokenKind::KwFunction:
            case TokenKind::KwGoto:
            case TokenKind::KwIf:
            case TokenKind::KwIn:
            case TokenKind::KwLocal:
            case TokenKind::KwNil:
            case TokenKind::KwNot:
            case TokenKind::KwOr:
            case TokenKind::KwRepeat:
            case TokenKind::KwReturn:
            case TokenKind::KwThen:
            case TokenKind::KwTrue:
            case TokenKind::KwUntil:
            case TokenKind::KwWhile: {
                string keyword =

                    lexer.idents[static_cast<int>(token.kind) -
                                 static_cast<int>(TokenKind::KwBreak)];
                output += keyword;

                break;
            }

            case TokenKind::Newline: {
                output += '\n';
                break;
            }

            default: {
                cout << "Unknown Token\n";
                break;
            }
        }
        output += ' ';
    }

    return output;
}

vector<Token> preprocessTokens(vector<Token> tokens) {
    vector<Token> output;

    for (size_t i = 0; i < tokens.size(); i++) {
        switch (tokens.at(i).kind) {
            case TokenKind::PlusEquals:
            case TokenKind::MinusEquals:
            case TokenKind::AsteriskEquals:
            case TokenKind::SlashEquals:
            case TokenKind::PercentEquals:
            case TokenKind::CaretEquals:
            case TokenKind::DotDotEquals: {
                // x += y
                // to x = x + y
                output.push_back((Token){.kind = TokenKind::Equals});

                vector<Token> x = getPreviousExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(x.begin()),
                              make_move_iterator(x.end()));

                output.push_back((Token){
                    .kind = lookupCompoundOpReplacement(tokens.at(i).kind)});

                vector<Token> y = getNextExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(y.begin()),
                              make_move_iterator(y.end()));

                i++;
                break;
            }

            case TokenKind::IntDiveEquals: {
                // x = \= y to
                // x = __idiv(x , y)
                output.push_back((Token){.kind = TokenKind::Equals});
                output.push_back((Token){
                    .kind = TokenKind::Iden,
                    .str = lookupBitwiseReplacement(TokenKind::IntegerDivide)});
                output.push_back((Token){.kind = TokenKind::Lparen});

                vector<Token> x = getPreviousExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(x.begin()),
                              make_move_iterator(x.end()));

                output.push_back((Token){.kind = TokenKind::Comma});

                vector<Token> y = getNextExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(y.begin()),
                              make_move_iterator(y.end()));

                output.push_back((Token){.kind = TokenKind::Rparen});

                i++;
                break;
            }

            case TokenKind::BitwiseAnd:
            case TokenKind::BitwiseOr:
            case TokenKind::BitwiseXor:
            case TokenKind::BitwiseNot:
            case TokenKind::ShiftLeft:
            case TokenKind::ShiftRight:
            case TokenKind::ArithShiftRight:
            case TokenKind::IntegerDivide: {
                // a >>b
                // to __shiftright(a,b)
                // a will already be present

                output.pop_back();
                output.push_back((Token){
                    .kind = TokenKind::Iden,
                    .str = lookupBitwiseReplacement(tokens.at(i).kind)});
                output.push_back((Token){.kind = TokenKind::Lparen});
                vector<Token> x = getPreviousExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(x.begin()),
                              make_move_iterator(x.end()));
                output.push_back((Token){.kind = TokenKind::Comma});
                vector<Token> y = getNextExpression(tokens, i);
                output.insert(output.end(), make_move_iterator(y.begin()),
                              make_move_iterator(y.end()));
                output.push_back((Token){.kind = TokenKind::Rparen});

                i++;
                break;
            }

            case TokenKind::KwIf: {
                // if (condition) then statement
                // to if condition then statement end
                output.push_back((Token){.kind = TokenKind::KwIf});
                if (tokens.at(i + 1).kind == TokenKind::Lparen) {
                    i++;

                    // TODO: change these to for loops
                    while (tokens.at(i).kind != TokenKind::Rparen) {
                        output.push_back(tokens.at(i));
                        i++;
                    }
                    output.push_back((Token){.kind = TokenKind::KwThen});
                    i++;
                    while (tokens.at(i).kind != TokenKind::Newline) {
                        output.push_back(tokens.at(i));
                        i++;
                    }
                    output.push_back((Token){.kind = TokenKind::KwEnd});
                }
                break;
            }

            case TokenKind::Question: {
                // ? "test" to
                // print("test")
                output.push_back(
                    (Token){.kind = TokenKind::Iden, .str = "print"});
                output.push_back((Token){.kind = TokenKind::Lparen});
                i++;
                while (tokens.at(i).kind != TokenKind::Newline) {
                    output.push_back(tokens.at(i));
                    i++;
                }
                output.push_back((Token){.kind = TokenKind::Rparen});

                break;
            }

            case TokenKind::BinNum: {
                int dec_int = stoi(tokens.at(i).str.substr(2), nullptr, 2);
                string int_str = to_string(dec_int);
                output.push_back(
                    (Token){.kind = TokenKind::DecNum, .str = int_str});

                break;
            }

            case TokenKind::HexNum: {
                if (tokens.at(i).str.find('.') == string::npos) {
                    output.push_back(tokens.at(i));
                    break;
                }

                string hex = tokens.at(i).str.substr(2);
                double decimal = 0;
                // TODO: convert fractional hex to double
                break;
            }

            default: {
                output.push_back(tokens.at(i));
                break;
            }
        }
    }
    return output;
}

vector<Token> getPreviousExpression(const vector<Token>& tokens, size_t i) {
    // TODO: going back to newline doesnt always work
    size_t initial_i = i;

    while (i > 0 && tokens.at(i).kind != TokenKind::Newline) {
        i--;
    }
    size_t final_i = i;
    vector<Token> expr(tokens.begin() + final_i + 1,
                       tokens.begin() + initial_i - 1);
    return expr;
}

vector<Token> getNextExpression(const vector<Token>& tokens, size_t i) {
    size_t initial_i = i;

    while (tokens.at(i).kind != TokenKind::Newline) {
        i++;
    }

    size_t final_i = i;

    vector<Token> expr(tokens.begin() + initial_i + 1,
                       tokens.begin() + final_i - 1);
    return expr;
}

TokenKind lookupCompoundOpReplacement(TokenKind kind) {
    switch (kind) {
        case TokenKind::PlusEquals:
            return TokenKind::Plus;
        case TokenKind::MinusEquals:
            return TokenKind::Minus;
        case TokenKind::AsteriskEquals:
            return TokenKind::Asterisk;
        case TokenKind::SlashEquals:
            return TokenKind::Slash;
        case TokenKind::PercentEquals:
            return TokenKind::Percent;
        case TokenKind::CaretEquals:
            return TokenKind::Caret;
        case TokenKind::DotDotEquals:
            return TokenKind::DotDot;

        default: {
            throw "Error: Unknown compound operator\n";
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
            throw "Error: Unknown bitwise operator\n";
        }
    }
}
