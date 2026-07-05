#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <vector>

using namespace std;

char Lexer::peek(Lexer lexer) { return lexer.rawLua.at(lexer.cursor); }

char Lexer::peekNext(Lexer lexer, int n) {
    // peek n digits forward
    // default n = 1
    return lexer.rawLua.at(lexer.cursor + n);
}

void Lexer::advance(Lexer lexer) { lexer.cursor++; }

vector<Token> Lexer::tokenise(Lexer lexer) {
    vector<Token> tokenArr;

    while (lexer.cursor != lexer.rawLua.length() - 1) {
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

                while (peek(lexer) != quote) {
                    if (peek(lexer) == '\\') {
                        lexer.cursor = lexer.cursor + 2;
                    }
                }
                int length = lexer.cursor - start;

                string str = lexer.rawLua.substr(start, length);
                token.kind = TokenKind::Str;
                token.str = str;

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

                while (isdigit(peek(lexer))) {
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
                break;
            }

            case '\n': {
                token.kind = TokenKind::Newline;
                break;
            }

            case '+': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::PlusEquals;
                else
                    token.kind = TokenKind::Plus;

                advance(lexer);
                break;
            }

            case '-': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::MinusEquals;
                else
                    token.kind = TokenKind::Minus;

                advance(lexer);
                break;
            }

            case '*': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::AsteriskEquals;
                else
                    token.kind = TokenKind::Asterisk;

                advance(lexer);
                break;
            }

            case '/': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::SlashEquals;
                else
                    token.kind = TokenKind::Slash;

                advance(lexer);
                break;
            }

            case '%': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::PercentEquals;
                else
                    token.kind = TokenKind::Percent;

                advance(lexer);
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
                    // is multi line str
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
                if (peekNext(lexer) == ':')
                    token.kind = TokenKind::DoubleColon;
                else
                    token.kind = TokenKind::Colon;

                advance(lexer);
                break;
            }

            case ',': {
                token.kind = TokenKind::Comma;

                advance(lexer);
                break;
            }

            case '.': {
                if (peekNext(lexer) != '.')
                    token.kind = TokenKind::Dot;
                else if (peekNext(lexer, 2) == '.')
                    token.kind = TokenKind::DotDotDot;
                else if (peekNext(lexer, 2) == '=')
                    token.kind = TokenKind::DotDotEquals;
                else
                    token.kind = TokenKind::DotDot;

                advance(lexer);
                break;
            }

            case '?': {
                token.kind = TokenKind::Question;
                advance(lexer);
                break;
            }

            case '=': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::EqualsEquals;
                else
                    token.kind = TokenKind::Equals;

                advance(lexer);
                break;
            }

            case '^': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::CaretEquals;
                else
                    token.kind = TokenKind::Caret;

                advance(lexer);
                break;
            }

            case '#': {
                token.kind = TokenKind::Hash;

                advance(lexer);
                break;
            }

            case '<': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::LessEquals;
                else if (peekNext(lexer) == '<')
                    token.kind = TokenKind::ShiftLeft;
                else
                    token.kind = TokenKind::Less;

                advance(lexer);
                break;
            }

            case '>': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::GreaterEquals;
                else if (peekNext(lexer) == '>' && peekNext(lexer, 2) == '>')
                    token.kind = TokenKind::ArithShiftRight;
                else if (peekNext(lexer) == '>' && !(peekNext(lexer, 2) == '>'))
                    token.kind = TokenKind::ShiftRight;
                else
                    token.kind = TokenKind::Greater;

                advance(lexer);
                break;
            }

            case '\\': {
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::IntDiveEquals;
                else
                    token.kind = TokenKind::IntegerDivide;

                advance(lexer);
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
                if (peekNext(lexer) == '=')
                    token.kind = TokenKind::NotEquals;
                else
                    token.kind = TokenKind::BitwiseNot;

                advance(lexer);
                break;
            }
        }

        tokenArr.push_back(token);
    }

    if (lexer.cursor == lexer.rawLua.length() - 1) {
        tokenArr.push_back((Token){.kind = TokenKind::End});
    }

    return tokenArr;
}
