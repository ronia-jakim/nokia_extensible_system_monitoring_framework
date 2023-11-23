//
// Created by sileanth on 23.11.23.
//

#include "parser.h"

#include <assert.h>
#include <inttypes.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#define STARTING_LENGTH 4

void erornij(char* error_msg) {
    fprintf(stderr, "%s", error_msg);
    exit(1);
}

int min(const int x, const int y) {
    if(x < y) return x;
    else return y;
}


enum unit {
    UNIT
};
typedef enum unit Unit; //representing 0 arg constructor nodes;

enum primitive_type {
    IntType,
    StringType
};
typedef enum primitive_type PrimitiveType;

struct node;
typedef struct node Node;
void free_node_tree(Node* node);

enum tag_type {
    STRING_NODE,
    INT_NODE,
    OBJECT_NODE,
    LIST_NODE,
    STRING_VALUE_NODE,
    IDENTIFIER_NODE,
    BOOL_NODE,
    NULL_NODE,
    PRIMITIVE_TYPE_NODE
};
typedef enum tag_type TagType;

struct object_node {
    int number_of_fields;
    int lists_capacity;
    char* *fields_names;
    Node* *fields_values;
};
typedef struct object_node ObjectNode;
ObjectNode new_object_node() {
    ObjectNode n = {
        .number_of_fields = 0,
        .lists_capacity = STARTING_LENGTH,
        .fields_names = malloc(sizeof(char*) * STARTING_LENGTH),
        .fields_values = malloc(sizeof(Node*) * STARTING_LENGTH),
    };
    return n;
}


void free_object_node_members(ObjectNode *object_node) {
    free(object_node->fields_names);
    free(object_node->fields_values);
    object_node->fields_names = NULL;
    object_node->fields_values = NULL;
}

void object_node_resize_lists_to_n(ObjectNode* object_node, const int n) {
    if(n == object_node->lists_capacity) return;

    char* *new_field_names = malloc(sizeof(char) * n);
    Node* *new_field_values = malloc(sizeof(Node*) * n);

    for(int i = 0; i < min(object_node->number_of_fields, n); i++) {
        new_field_names[i] = object_node->fields_names[i];
        new_field_values[i] = object_node->fields_values[i];
    }
    free_object_node_members(object_node);

    object_node->lists_capacity = n;
    object_node->fields_names = new_field_names;
    object_node->fields_values = new_field_values;
}

void object_node_push_field(ObjectNode* object_node, char* field_name, Node* field_value) {
    if(object_node->lists_capacity <= object_node->number_of_fields) {
        const int new_size = object_node->number_of_fields * 2 + 1;
        object_node_resize_lists_to_n(object_node, new_size);
    }

    object_node->fields_names[object_node->number_of_fields] = field_name;
    object_node->fields_values[object_node->number_of_fields] = field_value;
    object_node->number_of_fields++;
}

void object_node_shrink_lists_to_current_size(ObjectNode* object_node) {
    object_node_resize_lists_to_n(object_node, object_node->number_of_fields);
}




void object_node_iter(void (*f)(Node* n), ObjectNode* object_node) {
    for(int i = 0; i < object_node->number_of_fields; i++) {
        f(object_node->fields_values[i]);
    }
}


void free_object_node(ObjectNode* object_node) {
    object_node_iter(&free_node_tree, object_node);
    free_object_node_members(object_node);
    free(object_node);
}




struct list_node {
    int list_length;
    int list_capacity;
    Node* *elements;
};
typedef struct list_node ListNode;

struct node {
    TagType tag;
    union {
        char* string;
        char* delimited_string;
        int number;
        bool b;
        ObjectNode* object;
        ListNode* list;
        Unit null;
        PrimitiveType primitive_type;
    } data;
};
typedef struct node Node;
void print_node(Node* node);

ListNode new_list_node() {
    ListNode n = {
        .list_length = 0,
        .list_capacity = STARTING_LENGTH,
        .elements = malloc(sizeof(Node*) * STARTING_LENGTH),
    };
    return n;
}

void free_list_node_members(ListNode *list_node) {
    free(list_node->elements);
    list_node->elements = NULL;
}

void list_node_resize_list_to_n(ListNode* list_node, const int n) {
    if(n == list_node->list_capacity) return;

    Node* *new_list = malloc(sizeof(Node*) * n);

    for(int i = 0; i < min(list_node->list_length, n); i++) {
        new_list[i] = list_node->elements[i];
    }
    free_list_node_members(list_node);

    list_node->list_capacity = n;
    list_node->list_length = min(n, list_node->list_length);
    list_node->elements = new_list;
}

void list_node_push_field(ListNode* list_node, Node* value) {
    if(list_node->list_capacity <= list_node->list_length) {
        const int new_size = list_node->list_length * 2 + 1;
        list_node_resize_list_to_n(list_node, new_size);

    }

    list_node->elements[list_node->list_length] = value;

    list_node->list_length++;
}

void list_node_shrink_lists_to_current_size(ListNode* list_node) {
    list_node_resize_list_to_n(list_node, list_node->list_length);
}



void list_node_iter(void (*f)(Node* n), ListNode* list_node) {
    for(int i = 0; i < list_node->list_length; i++) {
        f(list_node->elements[i]);
    }
}

void free_list_node(ListNode* list_node) {
    list_node_iter(free_node_tree, list_node);
    free_list_node_members(list_node);
    free(list_node);
}




void shrink_single_node(Node* node) {
    if(node->tag == OBJECT_NODE) object_node_shrink_lists_to_current_size(node->data.object);
}

void node_iter(void (*f)(Node*), Node* node) {
    if(node->tag == OBJECT_NODE) object_node_iter(f, node->data.object);
    else if(node->tag == LIST_NODE) list_node_iter(f, node->data.list);
    else f(node);
}

void shrink_node_tree(Node* node) {
    if(node->tag == LIST_NODE) {
        list_node_iter(&shrink_node_tree, node->data.list);
        list_node_shrink_lists_to_current_size(node->data.list);
    }
    else if(node->tag == OBJECT_NODE) {
        object_node_iter(&shrink_node_tree, node->data.object);
        object_node_shrink_lists_to_current_size(node->data.object);
    }
}

void free_node_tree(Node* node) {
    if(node->tag == LIST_NODE) {
        free_list_node(node->data.list);
    } else if(node->tag == OBJECT_NODE) {
        free_object_node(node->data.object);
    } else if(node->tag == STRING_VALUE_NODE) {
        free(node->data.delimited_string);
    } else if(node->tag == STRING_NODE) {
        free(node->data.string);
    }

    free(node);
}

struct parser {
    struct lexer* l;
    Token* cur_token;
    Token* peek_token;
};
typedef struct parser Parser;

void parser_next_token(Parser* p) {
    p->cur_token = p->peek_token;
    Token* t = malloc(sizeof(Token));
    *t = lexer_next_token(p->l);
    p->peek_token = t;
}

Parser parser_new(Lexer *l) {
    Parser p;
    p.l = l;
    parser_next_token(&p);
    parser_next_token(&p);
    return p;
}

bool parser_cur_token_is(const Parser* p, const int token_type) {
    return p->cur_token->token_type == token_type;
}

bool parser_peek_token_is(const Parser* p, const int token_type) {
    return p->peek_token->token_type == token_type;
}

bool parser_advance_if_guess(Parser* p, const int token_type) {
    if(parser_cur_token_is(p, token_type)) {
        parser_next_token(p);
        return true;
    }
    return false;
}



Node* parser_create_primitive_value_node(const Token* t) {
    const int token_type = t->token_type;
    if(token_type == TOKEN_NULL) {
        Node* ptr = malloc(sizeof(Node));
        ptr->data.null = UNIT;
        ptr->tag = INT_NODE;
        return ptr;
    }
        if(token_type == TOKEN_STRING_TYPE) {
            Node* ptr = malloc(sizeof(Node));
            ptr->data.primitive_type = StringType;
            ptr->tag = PRIMITIVE_TYPE_NODE;
            return ptr;
        }
        if(token_type == TOKEN_INT_TYPE) {
            Node* ptr = malloc(sizeof(Node));
            ptr->data.primitive_type = IntType;
            ptr->tag = PRIMITIVE_TYPE_NODE;
            return ptr;
        }
        if(token_type == TOKEN_TRUE) {
            Node* ptr = malloc(sizeof(Node));
            ptr->data.b = true;
            ptr->tag = BOOL_NODE;
            return ptr;
        }
        if(token_type == TOKEN_FALSE) {
            Node* ptr = malloc(sizeof(Node));
            ptr->data.b = false;
            ptr->tag = BOOL_NODE;
            return ptr;
        }
        if(token_type == TOKEN_NUMBER) {

            int i = atoi(t->token_string);



            Node* ptr = malloc(sizeof(Node));
            ptr->data.number = i;
            ptr->tag = INT_NODE;

            return ptr;
        }
        if(token_type == TOKEN_DELIMITED_STRING) {
            char* string = malloc(t->token_length + 1);
            strcpy(string, t->token_string);
            Node* ptr = malloc(sizeof(Node));
            ptr->data.string = string;
            ptr->tag = STRING_VALUE_NODE;
            return ptr;
        }

        fprintf(stderr, "given token cant be converted to primitive value");
        exit(1);

    }

Node* parse_value(Parser *p) ;


Node* parse_object(Parser *p) {
    Node* n = malloc(sizeof(Node*));
    ObjectNode* on = malloc(sizeof(ObjectNode));
    *on = new_object_node();
    n->tag = OBJECT_NODE;
    n->data.object = on;
    parser_next_token(p);
    // ReSharper disable once CppDFAEndlessLoop
    while(true) {
        if(parser_advance_if_guess(p, TOKEN_RBRACE)) {
            break;
        };
        if(parser_cur_token_is(p, TOKEN_EOF)) {
            fprintf(stderr, "PARSING_ERROR: during parsing object,unexpected EOF");
            exit(1);
        }
        if(parser_cur_token_is(p, TOKEN_DELIMITED_STRING)) {
            char* field_name = malloc(p->cur_token->token_length + 1);
            strcpy(field_name, p->cur_token->token_string);
            parser_next_token(p);
            if(parser_advance_if_guess(p, TOKEN_SEMICOLON)) {
                Node* value = parse_value(p);
                object_node_push_field(on, field_name, value);
            } else {
                fprintf(stderr, "PARSING_ERROR: during parsing object, expected SEMICOLON got: %s", token_string(*p->cur_token));
                exit(1);
            }
        } else {
            fprintf(stderr, "PARSING_ERROR: during parsing object, expected DELIMITED_STRING got: %s", p->cur_token->token_string);
            exit(1);
        }
        if(parser_advance_if_guess(p, TOKEN_RBRACE)) {
            break;
        };
        if(parser_advance_if_guess(p, TOKEN_COMMA)) {
            continue;
        } else {
            fprintf(stderr, "PARSING_ERROR: during parsing object, expected COMMA or } got: %s", p->cur_token->token_string);
            exit(1);
        }
    }
    object_node_shrink_lists_to_current_size(on);
    return n;
}

Node* parse_list(Parser* p) {
    Node* n = malloc(sizeof(Node*));
    ListNode* nl = malloc(sizeof(ListNode));
    *nl = new_list_node();
    n->tag = LIST_NODE;
    n->data.list = nl;

    parser_next_token(p);
    // ReSharper disable once CppDFAEndlessLoop
    while(true) {

        int token_type = p->cur_token->token_type;
        if(token_type == TOKEN_EOF) {
            fprintf(stderr, "PARSING_ERROR: during parsing list,unexpected EOF");
            exit(1);
        }
        if(token_type == TOKEN_SQUARE_RBRACE) {
            parser_next_token(p);
            break;
        }

        list_node_push_field(nl, parse_value(p));

        token_type = p->cur_token->token_type;
        if(token_type == TOKEN_SQUARE_RBRACE) {
            parser_next_token(p);
            break;
        }

        if(token_type != TOKEN_COMMA) {
            fprintf(stderr, "PARSING_ERROR: during parsing list, expected ',' got %s", TOKEN_TYPE_TO_STRING[token_type]);
            exit(1);
        }
        parser_next_token(p);
    }
    list_node_shrink_lists_to_current_size(nl);
    return n;
}
Node* parse_value(Parser *p) {
    Node* node = malloc(sizeof(Node));

    const int token_type = p->cur_token->token_type;
    if(token_type == TOKEN_EOF) {
        fprintf( stderr, "PARSING_ERROR: END OF TOKENS!" );
        exit(1);
    };

    if(token_type == TOKEN_COMMA) {
        fprintf( stderr, "PARSING_ERROR: Expected value, got ','" );
        exit(1);
    }
    if(token_type == TOKEN_SEMICOLON) {
        fprintf( stderr, "PARSING_ERROR: Expected value, got ':'" );
        exit(1);
    }
    if(token_type == TOKEN_SQUARE_RBRACE) {
        fprintf( stderr, "PARSING_ERROR: Expected value, got ']'" );
        exit(1);
    }

    if(token_type == TOKEN_RBRACE) {
        fprintf( stderr, "PARSING_ERROR: Expected value, got '}'" );
        exit(1);
    }
    if(token_type == TOKEN_LBRACE) {
        return parse_object(p);
    }
    if(token_type == TOKEN_SQUARE_LBRACE) {
        return parse_list(p);
    }
    if(token_type == TOKEN_STRING_TYPE) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    }

    if(token_type == TOKEN_INT_TYPE) {

        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    }
    if(token_type == TOKEN_NULL) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    }
    if(token_type == TOKEN_TRUE) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    };
    if(token_type == TOKEN_FALSE) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    };
    if(token_type == TOKEN_NUMBER) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    };
    if(token_type == TOKEN_DELIMITED_STRING) {
        node = parser_create_primitive_value_node(p->cur_token);
        parser_next_token(p);
        return node;
    };

    fprintf(stderr, "PARSING ERROR: unmatched token: %s", token_string(*p->cur_token));
    exit(1);





}

void print_node(Node* node) {

    switch(node->tag) {
        case INT_NODE:
            if(node->tag == INT_NODE)

            printf("%d", node->data.number);
            break;
        case STRING_VALUE_NODE:
            printf("%s", node->data.string);
            break;

        case BOOL_NODE:
            printf("%d", node->data.b);
            break;

        case PRIMITIVE_TYPE_NODE:
            if(node->data.primitive_type == IntType)
                printf("INT_TYPE");
            else if(node->data.primitive_type == StringType)
                printf("STRING_TYPE");
            break;

        case NULL_NODE:
            printf("null");
            break;

        case OBJECT_NODE:
            printf("{");
            for(int i = 0; i < node->data.object->number_of_fields; i++) {
                printf("%s: ", node->data.object->fields_names[i]);
                print_node(node->data.object->fields_values[i]);
                printf(",");
            }
            printf("}");
            break;

        case LIST_NODE:
            printf("[");
            for(int i = 0; i < node->data.list->list_length; i++) {
                print_node(node->data.list->elements[i]);
                printf(", ");
            }
            printf("]");
            break;

        default:
            fprintf(stderr, "unhandled node variant in print_node");
            exit(1);
    }
}




void validate(Node* node) {
    switch(node->tag) {
        case INT_NODE:
            erornij("numerały nie moga się pojawiać w definicji formatu danych");
        case STRING_VALUE_NODE:
            erornij("Wartości stringów nei moga się pojawiac w formacie danych");
        case BOOL_NODE:
            erornij("Wartości Boolów nei moga się pojawiac w formacie danych");
        case NULL_NODE:
            erornij("Wartości null nei moga się pojawiac w formacie danych");
        case OBJECT_NODE:
            object_node_iter(&validate, node->data.object);
            break;
        case LIST_NODE:
            if(node->data.list->list_length != 1)
                erornij("W formacie danych typ listy oznacamy jak listę 1 elementową: \"[INT\"");
            list_node_iter(&validate, node->data.list);
            break;
        default:
            break;

    }
}

struct data_desc {
    Node* desc;
    char* name;
};
typedef struct data_desc DataDesc;


bool check_unique( DataDesc* ddesc, int length) {
    for(int i = 0; i < length; i++) {
        for(int j = i + 1; j < length; j++) {
            if(!strcmp(ddesc[i].name, ddesc[j].name)) {
                fprintf(stderr, "Kolizja nazw: %s\n%s\n", ddesc[i].name,ddesc[j].name);
                return false;
            }
        }
    }
    return true;
}

bool gen_table(Node* config,  DataDesc* ddesc, int* length) {
    if(config->tag != OBJECT_NODE) erornij("W configu na toplevelu musi znajdować się obiekt");
    validate(config);
    *length = config->data.object->number_of_fields;
    ddesc = malloc(sizeof(DataDesc) * *length );
    for(int i = 0; i < *length; i++) {
        ddesc[i].name = config->data.object->fields_names[i];
        ddesc[i].desc = config->data.object->fields_values[i];
    }
    return check_unique(ddesc, *length);
}


void test_parser() {

    char input_string[] = "{ \" a\" : [ INT ], \"b\" : STRING }";
    Lexer l = new_lexer(input_string);
  //  print_all_tokens(&l);
  //  return 0;
    Parser p = parser_new(&l);

    Node* n = parse_value(&p);
    print_node(n);
    DataDesc* data_desc_table;
    int* length = malloc(sizeof(int));
    gen_table(n, data_desc_table, length);



}