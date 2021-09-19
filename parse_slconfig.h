#include "memory.h"
#include "token.h"


struct Node {
    char* name;
    u8 id;
    u8 clock_prio;
    char* ip_0;
    char* ip_1;
    char* port_loc;
    char* port_lan;
};

struct App {
    char* name;
    StringList nodes;
    char* state;
};

Node ParseNode(Tokenizer* tokenizer, StackAllocator* stack) {
    Token token;
    Node node = {};

    u8 fields_parsed = 0;
    bool pcheck = false;
    bool parsing = true;
    while (fields_parsed < 7) {
        token = GetToken(tokenizer);

        if (TokenEquals(&token, "Name")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&node.name, &token, stack);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "ID")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            // NOTE: assuming the format 0xab for hex number ab ..
            token = GetToken(tokenizer);
            node.id = strtol(token.text, NULL, 16);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "clock_prio")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            node.clock_prio = (u8) atoi(token.text);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "network_address_0")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&node.ip_0, &token, stack);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "network_address_1")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&node.ip_1, &token, stack);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "port_loc")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&node.port_loc, &token, stack);

            ++fields_parsed;
        } 
        else if (TokenEquals(&token, "port_lan")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&node.port_lan, &token, stack);

            ++fields_parsed;
        }
    }

    return node;
}

App ParseApp(Tokenizer* tokenizer, StackAllocator* stack) {
    Token token;
    App app = {};

    u8 fields_parsed = 0;
    bool pcheck = false;
    bool parsing = true;
    while (fields_parsed < 3) {
        token = GetToken(tokenizer);

        if (TokenEquals(&token, "Name")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&app.name, &token, stack);

            ++fields_parsed;
        } else 
        if (TokenEquals(&token, "Node")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);
            pcheck = RequireToken(tokenizer, NULL, TOK_LSBRACK);
            assert(pcheck == true);

            ParseAllocCommaSeparatedListOfStrings(&app.nodes, tokenizer, stack);

            pcheck = RequireToken(tokenizer, NULL, TOK_RSBRACK);
            assert(pcheck == true);

            ++fields_parsed;
        } else 
        if (TokenEquals(&token, "State")) {
            pcheck = RequireToken(tokenizer, NULL, TOK_COLON);
            assert(pcheck == true);
            pcheck = RequireToken(tokenizer, NULL, TOK_LSBRACK);
            assert(pcheck == true);

            token = GetToken(tokenizer);
            AllocStringField(&app.state, &token, stack);

            pcheck = RequireToken(tokenizer, NULL, TOK_RSBRACK);
            assert(pcheck == true);

            ++fields_parsed;
        }
    }

    return app;
}

enum ParseMode {
    IDLE,
    NODES,
    APPS
};

void TestParseConfig() {
    char* text = LoadFile((char*) "config.yaml", true);
    if (text == NULL) {
        printf("could not load file");
        exit(1);
    }

    Tokenizer tokenizer = {};
    tokenizer.Init(text);
    ParseMode parsing_mode;

    // read the number of nodes / apps present
    u8 num_nodes = 0;
    u8 num_apps = 0;

    parsing_mode = IDLE;
    bool parsing = true;
    while (parsing) {
        Token token = GetToken(&tokenizer);
        switch ( token.type ) {
            case TOK_ENDOFSTREAM: {
                parsing = false;
            } break;

            case TOK_DASH: {
                if (parsing_mode == NODES) {
                    ++num_nodes;
                } else
                if (parsing_mode == APPS) {
                    ++num_apps;
                }
            } break;

            case TOK_IDENTIFIER: {
                if (TokenEquals(&token, "NODE") && RequireToken(&tokenizer, NULL, TOK_COLON)) {
                    parsing_mode = NODES;
                } else 
                if (TokenEquals(&token, "APP") && RequireToken(&tokenizer, NULL, TOK_COLON)) {
                    parsing_mode = APPS;
                }
            } break;

            default: {
            } break;
        }
    }

    StackAllocator stack(SIXTEEN_K);
    List<Node> nodes;
    nodes.Init(stack.Alloc(sizeof(Node) * num_nodes));
    List<App> apps;
    apps.Init(stack.Alloc(sizeof(App) * num_apps));

    tokenizer.Init(text);
    parsing_mode = IDLE;
    parsing = true;
    while (parsing) {
        Token token = GetToken(&tokenizer);

        switch ( token.type ) {
            case TOK_ENDOFSTREAM: {
                parsing = false;
            } break;

            case TOK_DASH: {
                if (parsing_mode == NODES) {
                    Node node = ParseNode(&tokenizer, &stack);
                    nodes.Add(&node);
                } else
                if (parsing_mode == APPS) {
                    App app = ParseApp(&tokenizer, &stack);
                    apps.Add(&app);
                }
            } break;

            case TOK_IDENTIFIER: {
                if (TokenEquals(&token, "NODE") && RequireToken(&tokenizer, NULL, TOK_COLON)) {
                    parsing_mode = NODES;
                } else
                if (TokenEquals(&token, "APP") && RequireToken(&tokenizer, NULL, TOK_COLON)) {
                    parsing_mode = APPS;
                }
            } break;

            default: {
            } break;
        }
    }
}
