#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char LBRACE[] = "{";
const char RBRACE[] = "{";
const char SEMICOLON[] = ":";
const char COMMA[] = ",";
const char SQUARE_LBRACE[] = "[";
const char SQUARE_RBRACE[] = "]";
const char DELIMITED_STRING[] = "DELIMITED_STRING";
const char NUMBER[] = "NUMBER";
const char TEOF[] = "EOF";

struct token {
    const char* token_type;
    char* token_string;
    int token_length;
};
typedef struct token Token;

void free_token_members(Token t) {
    free(t.token_string);
    t.token_string = NULL;
}

char* token_string(const Token t) {
    char temp_res[strlen(t.token_string) * 3 + strlen(t.token_type) + 10];
    sprintf(temp_res, "TOKEN_TYPE: %s\nTOKEN_LENGTH: %i\nTOKEN:\n%s", t.token_type, t.token_length, t.token_string);
    char* res = malloc(sizeof(char) * (strlen(temp_res) + 1));
    strcpy(res, temp_res);
    return res;
}
struct lexer {
    char* input;
    int input_length;

    int position;
    int read_postion;

    char current_char;
};
typedef struct lexer Lexer;
void free_lexer_members(Lexer l) {
    free(l.input);
    l.input = NULL;
}

void lexer_read_char(Lexer *l) {
    if(l->read_postion >= l->input_length) {
        l->current_char = 0;
    } else {
        l->current_char = l->input[l->read_postion];
    }
    l->position = l->read_postion;
    l->read_postion += 1;
}

Lexer new_lexer(char* input) {
    Lexer l = {
        .input = input,
        .input_length = strlen(input),
        .position = 0,
        .read_postion = 0,
        .current_char = 0,
    };
    l.input = input;

    lexer_read_char(&l);
    return l;
}

Token new_token(const char* token_type, char* token_string) {
    const Token t = {
        .token_type = token_type,
        .token_string = token_string,
        .token_length = strlen(token_string)
    };
    return t;
}

Token new_simple_token(const char* token_type) {
    char* token_string = malloc(sizeof(char) * (strlen(token_type) + 1));
    strcpy(token_string, token_type);
    return new_token(token_type, token_string);
}

void lexer_eat_whitespace(Lexer *l) {
    while((l->current_char == ' ' || l->current_char == '\n') && l->current_char != 0) {
        lexer_read_char(l);
    }
}

char* lexer_copy_string(const Lexer *l, const int starting_pos, const int length) {
    char* new_string = malloc((sizeof(char) * (length + 1)));
    strncpy(new_string, l->input + starting_pos, length);
    new_string[length] = '\0';

    return new_string;
}

Token lexer_read_delimited_string(Lexer *l) {
    const int starting_pos = l->position;
    do {
        lexer_read_char(l);
    } while((l->current_char != '"') && (l->current_char != 0));
    if(l->current_char != '"') {
        printf("LEXING ERROR: MISSING CLOSING \"");
        exit(42);
    }
    lexer_read_char(l); // consume closing '"'


    Token t;
    t.token_type = DELIMITED_STRING;
    t.token_length = (l->position - starting_pos );
    char* token_string = lexer_copy_string(l, starting_pos, t.token_length);


    t.token_string = token_string;

    return t;
}

Token lexer_read_number(Lexer *l) {
    const int starting_pos = l->position;
    do {
        lexer_read_char(l);
    } while(isdigit(l->current_char));
    const int token_length = (l->position - starting_pos );
    char* token_string = lexer_copy_string(l, starting_pos, token_length);

    if(strlen(token_string) == 1 && token_string[0] == '-') {
        printf("LEXING ERROR: \"-\" is not a valid number");
        exit(42);
    }

    const Token t = {
        .token_string = token_string,
        .token_type = NUMBER,
        .token_length = token_length
    };
    return t;
}


Token lexer_next_token(Lexer *l) {
    Token t;

    lexer_eat_whitespace(l);
    switch(l->current_char) {
        case '{':
            t = new_simple_token(LBRACE);
            break;
        case '}':
            t = new_simple_token(RBRACE);
            break;
        case '[':
            t = new_simple_token(SQUARE_LBRACE);
            break;
        case ']':
            t = new_simple_token(SQUARE_RBRACE);
            break;
        case ':':
            t = new_simple_token(SEMICOLON);
            break;
        case '"':
            return lexer_read_delimited_string(l);
        default:
            if(l->current_char == '-' || isdigit(l->current_char)) {
                return lexer_read_number(l);
            }
            t = new_simple_token(TEOF);
    }
    lexer_read_char(l);
    return t;
}


void lexer_handle_all(Lexer *l, void (*handlerPrt)(const Token)) {
    Token t;
    do {
        t = lexer_next_token(l);
        handlerPrt(t);
        free_token_members(t);
    } while(t.token_type != TEOF);
}

void lexer_handle_all_take_ownership(Lexer *l, void (*handlerPrt)(Token)) {
    Token t;
    do {
        t = lexer_next_token(l);
        handlerPrt(t);
    } while(t.token_type != TEOF);
}

void handler(const Token t) {
    printf("%s\n\n", token_string(t));
}

void print_all_tokens(Lexer *l) {
    lexer_handle_all(l, &handler);
}



int main() {
    char input_string[] = "\"123\"\"as\"  425 13\"ab\" -12 -3";
    Lexer l = new_lexer(input_string);

    print_all_tokens(&l);


}

