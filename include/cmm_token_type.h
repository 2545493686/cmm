#if !defined(___CMM_TOKEN_TYPE_H)
#define ___CMM_TOKEN_TYPE_H

typedef enum 
{
    Error,
    If, // if
    Return, // return
    LeftBracket, // (
    RightBracket, // )
    LeftBrace, // {
    RightBrace, // }
    Equal, // =
    Minus, // -
    Plus, // +
    Star, // *
    Slash, // /
    LessThan, // <
    GreaterThan, // >
    EOL, // ;
    Comma, // ,
    Identifier, 
    Integer,
} cmm_token_type;

#endif