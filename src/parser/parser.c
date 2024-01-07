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



        /*    Node* ptr = malloc(sizeof(Node));
            ptr->data.number = 42;
            ptr->tag = INT_NODE;

            return ptr; */
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

char * strcatt(char *s1, const char *s2)
{
    const size_t a = strlen(s1);
    const size_t b = strlen(s2);
    const size_t size_ab = a + b + 1;
  
    char* res = malloc(size_ab);
    res[0] = '\0';
    strcat(res, s1);
    strcat(res, s2);

    return res;
}

char* sprint_node(Node* node) {
    char* res = "";
    switch(node->tag) {
        case INT_NODE: {

            int len = snprintf(NULL,0, "%d", node->data.number);

            res = malloc(len + 2);
            sprintf(res, "%d", node->data.number);
            return res;
        }
        case STRING_VALUE_NODE: {
            int len = snprintf(NULL,0, "%s", node->data.string);
            res = malloc(len + 2);
            sprintf(res, "%s", node->data.string);
            return res;
        }
        case BOOL_NODE: {

    fprintf(stderr, "siema3");

            int len = snprintf(NULL,0, "%d", node->data.b);
            res = malloc(len + 2);
            return res;
        }
        case PRIMITIVE_TYPE_NODE: {
            if(node->data.primitive_type == IntType){
              res = "INT_TYPE";
              return res;
            }
            else if(node->data.primitive_type == StringType) {
                res = "STRING_TYPE";
                return res;
            }
                                  }
        case NULL_NODE: {
           res = "NULL";
           return res;
        }
        case OBJECT_NODE: {
            res = strcatt(res, "{");
            for(int i = 0; i < node->data.object->number_of_fields; i++) {
                res = strcatt(res, node->data.object->fields_names[i]);
                res = strcatt(res, ": ");
                res = strcatt(res, sprint_node(node->data.object->fields_values[i]));
                res = strcatt(res, ", ");
            }
            res = strcatt(res, "}");
            return res;
        }
        case LIST_NODE: {
            res = strcatt(res, "[");
            for(int i = 0; i < node->data.list->list_length; i++) {
                res = strcatt(res, sprint_node(node->data.list->elements[i]));
                res =strcatt(res, ", ");
            }
            res = strcatt(res, "]");
            return res;
        }
        default:
            fprintf(stderr, "unhandled node variant in print_node");
            exit(1);
    }
    return res;
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




void validate_data_desc(Node* node) {
    switch(node->tag) {
        case INT_NODE:
            erornij("numerały nie moga się pojawiać w definicji formatu danych");
        case STRING_VALUE_NODE:
            erornij("Wartości stringów nie moga się pojawiac w formacie danych");
        case BOOL_NODE:
            erornij("Wartości Boolów nie moga się pojawiac w formacie danych");
        case NULL_NODE:
            erornij("Wartości null nie moga się pojawiac w formacie danych");
        case OBJECT_NODE:
            object_node_iter(&validate_data_desc, node->data.object);
            break;
        case LIST_NODE:
            if(node->data.list->list_length != 1)
                erornij("W formacie danych typ listy oznacamy jak listę 1 elementową: \"[INT\"");
            list_node_iter(&validate_data_desc, node->data.list);
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

DataDesc* gen_table(Node* config, int* length) {
    if(config->tag != OBJECT_NODE) erornij("W configu na toplevelu musi znajdować się obiekt");
    validate_data_desc(config);
    *length = config->data.object->number_of_fields;
    DataDesc* ddesc = malloc(sizeof(DataDesc) * (*length + 4) );
    for(int i = 0; i < *length; i++) {
        ddesc[i].name = config->data.object->fields_names[i];
        ddesc[i].desc = config->data.object->fields_values[i];
    }
    if(!check_unique(ddesc, *length))return NULL;
    return ddesc;
}




bool validate_data_with_desc(Node* data, Node* desc) {
  if(desc->tag == PRIMITIVE_TYPE_NODE) {
    if(desc->data.primitive_type == IntType) {
      return (data->tag == INT_NODE);
    }
    if(desc->data.primitive_type == StringType) {
      return (data->tag == STRING_VALUE_NODE);
    }
    return false;
  }
  if(desc->tag == OBJECT_NODE) {
    if(data->tag != OBJECT_NODE) {
      fprintf(stderr, "oczekiwano obiektu\n");
      return false;
    }
    if(data->data.object->number_of_fields != desc->data.object->number_of_fields) return false;
    for(int i = 0; i < data->data.object->number_of_fields; i++) {
      if(strcmp(data->data.object->fields_names[i], desc->data.object->fields_names[i])) {
        fprintf(stderr, "nie ma pola o nazwie %s\n", desc->data.object->fields_names[i]);
        fprintf(stderr, "pole ma nazwe %s\n", data->data.object->fields_names[i]);
        return false;
      }
      if(!validate_data_with_desc(data->data.object->fields_values[i], desc->data.object->fields_values[i])) {
        fprintf(stderr, "pole %s nie zgadza sie z data_desc\n", desc->data.object->fields_names[i]);
        return false;
      }
    }
    return true;
  }
  if(desc->tag == LIST_NODE) {
    if(data->tag != LIST_NODE) return false;
    for(int i = 0; i < data->data.list->list_length; i++) {
      if(!validate_data_with_desc(data->data.list->elements[i], desc->data.list->elements[0])) {
        fprintf(stderr, "%i element list ma zly typ\n", i);
        return false;
      }
    }
    return true;

  }



  return true;
}


int validate_sended_data(Node* sended_data, DataDesc* data_desc_table, int data_desc_table_length) {
  int data_index;

  // check fif toplevel node is object
  if(sended_data->tag != OBJECT_NODE) {
    fprintf(stderr, "%s", "toplevel node should be object in sended_data");
    return -1;
  }
  // get headers from toplevel node 
  Node* headers = NULL;
  for(int i = 0; i < sended_data->data.object->number_of_fields; i++) {
    if(!strcmp(sended_data->data.object->fields_names[i], "\"headers\"")) {
      headers = sended_data->data.object->fields_values[i];
      break;
    }
  }
  if(headers == NULL) return false;
  if(headers->tag != OBJECT_NODE) {
    fprintf(stderr, "%s", "headers should be object");
    return -1;
  }

// get node_id from headers
  Node* node_id_node = NULL;
  for(int i = 0; i < headers->data.object->number_of_fields; i++) {
    if(!strcmp(headers->data.object->fields_names[i], "\"node_id\"")) {
      node_id_node = headers->data.object->fields_values[i];
      break;
    }
  }
  if(node_id_node == NULL) return -1;
  if(node_id_node->tag != INT_NODE) return -1;
  int node_id = node_id_node->data.number;

// get node_id from headers
  Node* plugin_id_node = NULL;
  for(int i = 0; i < headers->data.object->number_of_fields; i++) {
    if(!strcmp(headers->data.object->fields_names[i], "\"plugin_id\"")) {
      plugin_id_node = headers->data.object->fields_values[i];
      break;
    }
  }
  if(plugin_id_node == NULL) return false;
  if(plugin_id_node->tag != INT_NODE) return false;
  int plugin_id = plugin_id_node->data.number;

 // get data_type from headers

  Node* data_type = NULL;
  for(int i = 0; i < headers->data.object->number_of_fields; i++) {
    if(!strcmp(headers->data.object->fields_names[i], "\"data_type\"")) {
      data_type = headers->data.object->fields_values[i];
      break;
    }
  }
  if(data_type == NULL) return -1;
  if(data_type->tag != STRING_VALUE_NODE) return -1;
  char* string_of_data_type = data_type->data.string;



  //get data_description for sended_data
  Node* data_desc = NULL;
  for(int i = 0; i < data_desc_table_length; i++) {
    if(!strcmp(string_of_data_type, data_desc_table[i].name)) {
      data_desc = data_desc_table[i].desc;
      data_index = i;
      break;
    }
  }
  if(data_desc == NULL) {

    fprintf(stderr, "%s", "nie znaleziono data description o tym name\n");
    return -1;
  }


  //get data_lit from toplevle node
  Node* data_list = NULL;
  for(int i = 0; i < sended_data->data.object->number_of_fields; i++) {
    if(!strcmp(sended_data->data.object->fields_names[i], "\"data_list\"")) {
      data_list = sended_data->data.object->fields_values[i];
      break;
    }
  }

  if(data_desc == NULL) return -1;
  if(data_list == NULL) return -1;
  if(data_list->tag != LIST_NODE) {
    return -1;
  }


  // validate each data_entry 
  for(int i = 0; i < data_list->data.list->list_length; i++) {
    Node* data_entry = data_list->data.list->elements[i];
    if(data_entry->tag != OBJECT_NODE) return -1;

    //get nodes from data_entry
    Node* time_node = NULL;
    Node* data_node = NULL;
    for(int j = 0; j < data_entry->data.object->number_of_fields; j++) {
      if(!strcmp("\"time\"", data_entry->data.object->fields_names[j])) {
       time_node = data_entry->data.object->fields_values[j]; 
      }
      if(!strcmp("\"data\"", data_entry->data.object->fields_names[j])) {
       data_node = data_entry->data.object->fields_values[j]; 
      }
    }

    // get time
    if(time_node == NULL) {
      fprintf(stderr, "w %i-tej data_entry nie ma pola time", i);
      return -1;
    }
    if(time_node->tag != INT_NODE) {
      fprintf(stderr, "w %i-tej data_entry pole time nie zawiera inta", i);
      return -1;
    }
    int time = time_node->data.number;


    // get data 
    if(data_node == NULL) return -1;
    if(!validate_data_with_desc(data_node, data_desc)) {
      fprintf(stderr, "w %i-tej data_entry pole data jest niezgodne z opisem danych", i);
      return -1;
    }
  }


  return data_index;
}

/*
  { "headers": {
      "node_id": 3,
      "plugin_id": 2,
      "data_type": "a",
    },
    "data_list": [
      {
        "time": 1,
        "data": //data described by data type
      }
    ]
  }
 
 */
/*
  { "headers": {
      "node_id": 3,
      "plugin_id": 2,
      "data_type": "a",
      "data_desc": 
    },
    "data_list": [
      {
        "time": 1,
        "data": //data described by data type
      }
    ]
  }
 
 
 
 */


Node* parse_node(char* text) {
    Lexer l = new_lexer(text);
    Parser p = parser_new(&l);
    Node* n = parse_value(&p);
    return n;

}

int parse_sended_data(char* sended_data, DataDesc* data_desc_table, int data_desc_table_length) {
  Lexer l = new_lexer(sended_data);
  Parser p = parser_new(&l);
  Node* n = parse_value(&p);
  return validate_sended_data(n, data_desc_table, data_desc_table_length);
}

Node* annotate_sended_data(char* sended_data, DataDesc* data_desc_table, int data_desc_table_length) {
  Lexer l = new_lexer(sended_data);
  Parser p = parser_new(&l);
  Node* n = parse_value(&p);
  int data_index = validate_sended_data(n, data_desc_table, data_desc_table_length);
  if(data_index < 0 || data_index >= data_desc_table_length) return NULL;
  
  Node* headers = NULL;
  for(int i = 0; i < n->data.object->number_of_fields; i++) {
    if(!strcmp(n->data.object->fields_names[i], "\"headers\"")) {
      headers = n->data.object->fields_values[i];
      break;
    }
  }
  object_node_push_field(headers->data.object, "data_desc", data_desc_table[data_index].desc);




  return n;
}



char* annotate_string_data(char* sended_data, char* data_desc) {
    Lexer l = new_lexer(data_desc);
    Parser p = parser_new(&l);
    Node* data_desc_node = parse_value(&p);
    int* length = malloc(sizeof(int));
    DataDesc* data_desc_table = gen_table(data_desc_node, length);

    Node* annotated_data = annotate_sended_data(sended_data, data_desc_table, *length);
    return sprint_node(annotated_data);
}


void test_annotate_parser() {
    char input_string_desc[] = "{ \"a\" : [ { \"first\": INT, \"second\": STRING } ], \"b\" : STRING , \"c\": { \"a\": [ STRING] }}";


    char* sended_data_b = "{ \"headers\": { \"node_id\": 7, \"plugin_id\":2, \"data_type\": \"b\" }, \"data_list\": [ { \"time\": 4, \"data\": \"abc\" }, { \"time\": 1, \"data\": \"abdc\" } ] }";
    printf("%s\n\n", annotate_string_data(sended_data_b, input_string_desc));

    char* sended_data_a = "{ \"headers\": { \"node_id\": 7, \"plugin_id\":2, \"data_type\": \"a\" }, \"data_list\": [ { \"time\": 4, \"data\": [       { \"first\" : 9, \"second\": \"ssd\"},     { \"first\" : 3, \"second\": \"ss\"}, ] } ] }    ";
    printf("%s", annotate_string_data(sended_data_a, input_string_desc));
}

void test_parser() {

    char input_string_desc[] = "{ \"a\" : [ { \"first\": INT, \"second\": STRING } ], \"b\" : STRING , \"c\": { \"a\": [ STRING] }}";
    Lexer l = new_lexer(input_string_desc);
    Parser p = parser_new(&l);
    Node* n = parse_value(&p);
    int* length = malloc(sizeof(int));
    DataDesc* data_desc_table = gen_table(n, length);



    char* sended_data_b = "{ \"headers\": { \"node_id\": 7, \"plugin_id\":2, \"data_type\": \"b\" }, \"data_list\": [ { \"time\": 4, \"data\": \"abc\" }, { \"time\": 1, \"data\": \"abdc\" } ] }";
    if(parse_sended_data(sended_data_b, data_desc_table, *length) >= 0) {
      printf("checked\n");
    } else {
      printf("wrong\n");
    }


    char* sended_data_a = "{ \"headers\": { \"node_id\": 7, \"plugin_id\":2, \"data_type\": \"a\" }, \"data_list\": [ { \"time\": 4, \"data\": [       { \"first\" : 9, \"second\": \"ssd\"},     { \"first\" : 3, \"second\": \"ss\"}, ] } ] }    ";
    if(parse_sended_data(sended_data_a, data_desc_table, *length)) {
      printf("checked\n");
    } else {
      printf("wrong\n");
    }

}
