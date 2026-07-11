#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <vector>

using namespace std;

char Lexer::peek(Lexer& lexer) {
    if (lexer.cursor >= lexer.rawLua.length()) return '\0';
    return lexer.rawLua.at(lexer.cursor);
}

char Lexer::peekNext(Lexer& lexer, int n) {
    // peek n digits forward
    // default n = 1
    if (lexer.cursor >= lexer.rawLua.length()) return '\0';
    return lexer.rawLua.at(lexer.cursor + n);
}

void Lexer::advance(Lexer& lexer, int n) {
    // advance n digits forward
    // default n = 1
    lexer.cursor = lexer.cursor + n;
}

vector<Token> Lexer::tokenise(Lexer lexer) {
    vector<Token> tokenArr;

    while (lexer.cursor < lexer.rawLua.length()) {
        Token token;

        switch (peek(lexer)) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_': {
                int start = lexer.cursor;

                while (isalnum(peek(lexer)) || peek(lexer) == '_') {
                    advance(lexer);
                }

                int length = lexer.cursor - start;
                string word = lexer.rawLua.substr(start, length);

                auto iter =
                    find(lexer.idents.begin(), lexer.idents.end(), word);

                if (iter == lexer.idents.end()) {
                    // an iden
                    token.kind = TokenKind::Iden;
                    token.str = word;
                } else {
                    // a keyword
                    int index = distance(lexer.idents.begin(), iter);
                    token.kind = static_cast<TokenKind>(
                        static_cast<int>(TokenKind::KwBreak) + index);
                }

                break;
            }

            // ' and "
            case '\'':
            case '\"': {
                char quote = peek(lexer);
                lexer.cursor++;
                int start = lexer.cursor;

                while (peek(lexer) != quote && peek(lexer) != '\0') {
                    if (peek(lexer) == '\\') {
                        lexer.cursor = lexer.cursor + 2;
                    } else {
                        advance(lexer);
                    }
                }
                int length = lexer.cursor - start;

                string str = lexer.rawLua.substr(start, length);
                token.kind = TokenKind::Str;
                token.str = str;
                advance(lexer);

                break;
            }
                // TODO: handle string literals

                // numbers:
                // decimals, hexadecimals, binary, 1e2
                // x.y, 0x, 0B/0b, 1.3e7

            case '0' ... '9': {
                bool is_sci = false;
                bool is_hex = false;
                bool is_bin = false;

                if (peek(lexer) == '0' && peekNext(lexer) == 'x') {
                    is_hex = true;
                } else if (peek(lexer) == '0' &&
                           (peekNext(lexer) == 'b' || peekNext(lexer) == 'B')) {
                    is_bin = true;
                }

                int start = lexer.cursor;

                while (isdigit(peek(lexer)) || peek(lexer) == 'e' ||
                       peek(lexer) == 'E' || peek(lexer) == '.') {
                    if (peek(lexer) == 'e') is_sci = true;
                    advance(lexer);
                }

                int length = lexer.cursor - start;
                string num = lexer.rawLua.substr(start, length);
                token.str = num;

                if (is_hex)
                    token.kind = TokenKind::HexNum;
                else if (is_bin)
                    token.kind = TokenKind::BinNum;
                else if (is_sci)
                    token.kind = TokenKind::SciNum;
                else
                    token.kind = TokenKind::DecNum;

                break;
            }

            case ' ':
            case '\t': {
                advance(lexer);
                continue;
            }

            case '\n': {
                token.kind = TokenKind::Newline;
                break;
            }

            case '+': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::PlusEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Plus;

                    advance(lexer);
                }
                break;
            }

            case '-': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::MinusEquals;
                    advance(lexer);
                }

                else if (peekNext(lexer) == '-' &&
                         !(peekNext(lexer, 2) == '[' &&
                           peekNext(lexer, 3) == '[')) {
                    // -- comments without [[

                    while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                        advance(lexer);
                    }
                    advance(lexer);

                    // TODO: add support for --[==[ ]==]

                } else if (peekNext(lexer) == '-' &&
                           (peekNext(lexer, 2) == '[' &&
                            peekNext(lexer, 3) == '[')) {
                    // -- comments with [[
                    advance(lexer, 3);
                    while (!(peek(lexer) == ']' && peekNext(lexer) == ']') &&
                           peek(lexer) != '\0') {
                        advance(lexer);
                    }

                    if (peek(lexer) != '\0') {
                        advance(lexer, 2);
                    }

                } else {
                    token.kind = TokenKind::Minus;

                    advance(lexer);
                }
                break;
            }

            case '*': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::AsteriskEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Asterisk;

                    advance(lexer);
                }
                break;
            }

            case '/': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::SlashEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Slash;

                    advance(lexer);
                }
                break;
            }

            case '%': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::PercentEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Percent;

                    advance(lexer);
                }
                break;
            }

            case '(': {
                token.kind = TokenKind::Lparen;

                advance(lexer);
                break;
            }

            case ')': {
                token.kind = TokenKind::Rparen;

                advance(lexer);
                break;
            }

            case '{': {
                token.kind = TokenKind::Lbrace;

                advance(lexer);
                break;
            }

            case '}': {
                token.kind = TokenKind::Rbrace;

                advance(lexer);
                break;
            }

            case '[': {
                if (peekNext(lexer) == '[') {
                    // is multi line string literal
                }
                token.kind = TokenKind::Lbracket;

                advance(lexer);
                break;
            }

            case ']': {
                token.kind = TokenKind::Rbracket;

                advance(lexer);
                break;
            }

            case ';': {
                token.kind = TokenKind::Semicolon;

                advance(lexer);
                break;
            }

            case ':': {
                if (peekNext(lexer) == ':') {
                    token.kind = TokenKind::DoubleColon;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Colon;

                    advance(lexer);
                }
                break;
            }

            case ',': {
                token.kind = TokenKind::Comma;

                advance(lexer);
                break;
            }

            case '.': {
                if (peekNext(lexer) != '.') {
                    token.kind = TokenKind::Dot;
                    advance(lexer);
                } else if (peekNext(lexer, 2) == '.') {
                    token.kind = TokenKind::DotDotDot;
                    advance(lexer, 3);
                } else if (peekNext(lexer, 2) == '=') {
                    token.kind = TokenKind::DotDotEquals;
                    advance(lexer, 3);
                } else {
                    token.kind = TokenKind::DotDot;
                    advance(lexer, 2);
                }

                break;
            }

            case '?': {
                token.kind = TokenKind::Question;
                advance(lexer);
                break;
            }

            case '=': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::EqualsEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Equals;

                    advance(lexer);
                }
                break;
            }

            case '^': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::CaretEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Caret;

                    advance(lexer);
                }
                break;
            }

            case '#': {
                token.kind = TokenKind::Hash;

                advance(lexer);
                break;
            }

            case '<': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::LessEquals;
                    advance(lexer, 2);
                } else if (peekNext(lexer) == '<') {
                    token.kind = TokenKind::ShiftLeft;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Less;

                    advance(lexer);
                }
                break;
            }

            case '>': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::GreaterEquals;
                    advance(lexer, 2);
                } else if (peekNext(lexer) == '>' &&
                           peekNext(lexer, 2) == '>') {
                    token.kind = TokenKind::ArithShiftRight;
                    advance(lexer, 3);
                } else if (peekNext(lexer) == '>' &&
                           !(peekNext(lexer, 2) == '>')) {
                    token.kind = TokenKind::ShiftRight;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::Greater;
                    advance(lexer);
                }
                break;
            }

            case '\\': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::IntDiveEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::IntegerDivide;

                    advance(lexer);
                }
                break;
            }

            case '&': {
                token.kind = TokenKind::BitwiseAnd;

                advance(lexer);
                break;
            }

            case '|': {
                token.kind = TokenKind::BitwiseOr;

                advance(lexer);
                break;
            }

            case '~': {
                if (peekNext(lexer) == '=') {
                    token.kind = TokenKind::NotEquals;
                    advance(lexer, 2);
                } else {
                    token.kind = TokenKind::BitwiseNot;

                    advance(lexer);
                }
                break;
            }

            default: {
                cerr << "Warning: Unexpected character passed to lexer:" << '\"'
                     << peek(lexer) << '\"' << '\n';
                advance(lexer);
                continue;
            }
        }

        tokenArr.push_back(token);
    }

    tokenArr.push_back((Token){.kind = TokenKind::End});

    return tokenArr;
}
