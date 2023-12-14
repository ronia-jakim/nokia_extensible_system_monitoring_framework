#ifndef LEXER_H
#define LEXER_H


// TOKEN
extern const int TOKEN_LBRACE;
extern const int TOKEN_RBRACE;
extern const int TOKEN_SEMICOLON;
extern const int TOKEN_COMMA;
extern const int TOKEN_SQUARE_LBRACE;
extern const int TOKEN_SQUARE_RBRACE ;
extern const int TOKEN_DELIMITED_STRING ;
extern const int TOKEN_UNKNOWN_IDENTIFIER;
extern const int TOKEN_NUMBER;
extern const int TOKEN_EOF;
extern const int TOKEN_TRUE ;
extern const int TOKEN_FALSE ;
extern const int TOKEN_NULL;
extern const int TOKEN_STRING_TYPE ;
extern const int TOKEN_INT_TYPE;
extern const char* const TOKEN_TYPE_TO_STRING[];

struct token {
    int token_type;
    char* token_string;
    int token_length;
};
typedef struct token Token;

void free_token_members(Token t);

char* token_string(const Token t);

// LEXER
struct lexer {
    char* input;
    int input_length;

    int position;
    int read_postion;

    char current_char;
};
typedef struct lexer Lexer;
void free_lexer_members(Lexer l);

Lexer new_lexer(char* input);

Token lexer_next_token(Lexer *l);

void lexer_handle_all(Lexer *l, void (*handlerPrt)(const Token));

void lexer_handle_all_take_ownership(Lexer *l, void (*handlerPrt)(Token));

void print_all_tokens(Lexer *l);

// TESTING
void test();

#endif //LEXER_H
