#include <string>
#include <vector>

enum class TokenKind {
    Iden,
    DecNum,
    HexNum,
    BinNum,
    SciNum,
    Str,

    Plus,
    Minus,
    Asterisk,
    Slash,
    Percent,
    Lparen,
    Rparen,
    Lbrace,
    Rbrace,
    Lbracket,
    Rbracket,
    Semicolon,
    Colon,
    Comma,
    Dot,
    Question,

    Equals,
    Caret,
    Hash,
    Less,
    Greater,

    PlusEquals,
    MinusEquals,
    AsteriskEquals,
    SlashEquals,
    PercentEquals,
    CaretEquals,
    DotDotEquals,

    NotEquals,      // ~= , !=
    IntegerDivide,  // '\'
    IntDiveEquals,  // '\='

    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,       // ^^
    BitwiseNot,       // ~
    ShiftLeft,        // <<
    ShiftRight,       // >>
    ArithShiftRight,  // >>>

    EqualsEquals,
    LessEquals,
    GreaterEquals,
    DotDot,
    DotDotDot,
    DoubleColon,

    Flr,
    Ceil,
    Rnd,
    Max,
    Min,

    Sin,
    Cos,

    Add,
    Del,
    All,
    Foreach,

    Sub,
    Chr,
    Ord,

    KwBreak,
    KwDo,
    KwElse,
    KwElseif,
    KwEnd,
    KwFalse,
    KwFor,
    KwFunction,
    KwGoto,
    KwIf,
    KwIn,
    KwLocal,
    KwNil,
    KwNot,
    KwOr,
    KwRepeat,
    KwReturn,
    KwThen,
    KwTrue,
    KwUntil,
    KwWhile,

    Newline,
    End,
};

struct Token {
    TokenKind kind;
    std::string str;
};

class Lexer {
   public:
    std::string rawLua;
    unsigned int cursor = 0;
    unsigned int line = 0;
    unsigned int columb = 0;

    std::vector<std::string> idents = {
        "break",    "do",     "else",   "elseif", "end",   "false", "for",
        "function", "goto",   "if",     "in",     "local", "nil",   "not",
        "or",       "repeat", "return", "then",   "true",  "until", "while",
    };

    std::vector<Token> tokenise(Lexer lexer);

    char peek(Lexer &lexer);
    char peekNext(Lexer &lexer, int n = 1);

    void advance(Lexer &lexer, int n = 1);
};
