#ifndef __TOKEN_H__
#define __TOKEN_H__


#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "various.h"
#include "memory.h"


/**
* Light-weight, multi-purpose Tokenizer code meant for copy-paste and modification.
* Implementation roughly following handmade hero.
*/

enum TokenType {
  TOK_UNKNOWN, // catch-all for things that aren't defined yet

  TOK_LBRACK, // (
  TOK_RBRACK,
  TOK_LBRACE, // {
  TOK_RBRACE,
  TOK_LSBRACK, // [
  TOK_RSBRACK,
  TOK_LEDGE,
  TOK_REDGE,
  TOK_POUND,
  TOK_ASTERISK,
  TOK_COMMA,
  TOK_DOT,
  TOK_SLASH,
  TOK_DASH,
  TOK_PLUS,
  TOK_COLON,
  TOK_SEMICOLON,
  TOK_ASSIGN,
  TOK_EXCLAMATION,
  TOK_TILDE,
  TOK_OR,
  TOK_AND,
  TOK_PERCENT,

  TOK_CHAR,
  TOK_STRING,
  TOK_NUMERIC,
  TOK_IDENTIFIER,

  TOK_ENDOFSTREAM,
};

struct Tokenizer {
  char* at;
};

struct Token {
  TokenType type;
  char* text;
  u16 len;
};

inline
bool IsEndOfLine(char c) {
  return c == '\n' || c == '\r';
}

inline
bool IsWhitespace(char c) {
  return
    c == ' ' ||
    c == '\t' ||
    c == '\v' ||
    c == '\f' ||
    IsEndOfLine(c);
}

inline
bool IsNumeric(char c) {
  return (c >= '0') && (c <= '9');
}

inline
bool IsAlphaOrUnderscore(char c) {
  bool result = 
    (c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
  return result;
}

void EatCppStyleComment(Tokenizer* tokenizer) {
  for(;;) {
    if (tokenizer->at[0] == '\0')
      return;

    if (tokenizer->at[1] == '\0') {
      ++tokenizer->at;
      return;
    }

    if (tokenizer->at[0] == '\n') {
      ++tokenizer->at;
      return;
    }

    ++tokenizer->at;
  }
}

void EatCStyleComment(Tokenizer* tokenizer) {
  tokenizer->at += 2; // assuming at = "/* ... */"
  for(;;) {
    if (tokenizer->at[0] == '\0')
      return;

    if (tokenizer->at[1] == '\0') {
      ++tokenizer->at;
      return;
    }

    if (tokenizer->at[0] == '*' && tokenizer->at[1] && tokenizer->at[1] == '/') {
      tokenizer->at += 2;
      return;
    }

    ++tokenizer->at;
  }
}

void EatWhiteSpacesAndComments(Tokenizer* tokenizer) {
  for (;;) {
    if (IsWhitespace(tokenizer->at[0])) {
      ++tokenizer->at;
    }

    else if (tokenizer->at[0] == '/') {
      if (tokenizer->at[1]) {
        if (tokenizer->at[1] == '/') { // C++-style comment
          EatCppStyleComment(tokenizer);
        }
        else if (tokenizer->at[1] == '*') { // C-style comment
          EatCStyleComment(tokenizer);
        }
        else {
          // sometimes, a slash is just a slash
          return;
        }
      }
      else {
        // end of stream
        ++tokenizer->at;
        return;
      }
    }

    else if (tokenizer->at[0] == '#') { // pythonic | matlabish comment
      EatCppStyleComment(tokenizer);
    }

    else {
      break;
    }
  }
}

// meant to be used with static strings, e.g. TokenEquals(tok, "hest")
bool TokenEquals(Token* token, char* match) { 
  char* at = match;
  for (int idx = 0; idx < token->len; idx++, at++) {
    if (token->text[idx] != *at) {
      return false;
    }
  }
  bool result = (*at == 0);
  return true;
}

Token GetToken(Tokenizer* tokenizer) {
  EatWhiteSpacesAndComments(tokenizer);

  Token token = {};
  token.text = tokenizer->at;
  token.len = 1;

  char c = tokenizer->at[0];
  ++tokenizer->at;
  switch (c) {

    case '\0': token.type = TOK_ENDOFSTREAM; break;
    case '(' : token.type = TOK_LBRACK; break;
    case ')' : token.type = TOK_RBRACK; break;
    case '{' : token.type = TOK_LBRACE; break;
    case '}' : token.type = TOK_RBRACE; break;
    case '[' : token.type = TOK_LSBRACK; break;
    case ']' : token.type = TOK_RSBRACK; break;
    case '<' : token.type = TOK_LEDGE; break;
    case '>' : token.type = TOK_REDGE; break;
    case '#' : token.type = TOK_POUND; break;
    case '*' : token.type = TOK_ASTERISK; break;
    case ',' : token.type = TOK_COMMA; break;
    case '.' : token.type = TOK_DOT; break;
    case '/' : token.type = TOK_SLASH; break;
    case '-' : token.type = TOK_DASH; break;
    case '+' : token.type = TOK_PLUS; break;
    case ':' : token.type = TOK_COLON; break;
    case ';' : token.type = TOK_SEMICOLON; break;
    case '=' : token.type = TOK_ASSIGN; break;
    case '!' : token.type = TOK_EXCLAMATION; break;
    case '~' : token.type = TOK_TILDE; break;
    case '|' : token.type = TOK_OR; break;
    case '&' : token.type = TOK_AND; break;
    case '%' : token.type = TOK_PERCENT; break;

    case '"' : {

      token.type = TOK_STRING;

      while (tokenizer->at[0] != '\0' && tokenizer->at[0] != '"') {
        if (tokenizer->at[0] == '\\' && tokenizer->at[1]) {
          ++tokenizer->at;
        }
        ++tokenizer->at;
      }

      if (tokenizer->at[0] == '"') {
        ++tokenizer->at;
      }

      token.len = tokenizer->at - token.text;
    } break;

    case '\'' : {

      token.type = TOK_CHAR;
  
      while (tokenizer->at[0] != '\0' && tokenizer->at[0] != '\'') {
        if (tokenizer->at[0] == '\\' && tokenizer->at[1]) {
          ++tokenizer->at;
        }
        ++tokenizer->at;
      }

      if (tokenizer->at[0] == '\'') {
        ++tokenizer->at;
      }

      token.len = tokenizer->at - token.text;
    } break;

    default : {

      if (IsAlphaOrUnderscore(c)) {
        token.type = TOK_IDENTIFIER;

        while (
          tokenizer->at[0] != '\0' &&
          (IsAlphaOrUnderscore(tokenizer->at[0]) || IsNumeric(tokenizer->at[0]))) {
          ++tokenizer->at;
        }

        token.len = tokenizer->at - token.text;
      }
      else if (IsNumeric(c)) {
        token.type = TOK_NUMERIC;

        while (tokenizer->at[0] != '\0' && IsNumeric(tokenizer->at[0])) {
          ++tokenizer->at;
        }

        token.len = tokenizer->at - token.text;
      }
      else {
        token.type = TOK_UNKNOWN;
      }
    }

  }
  return token;
}

// TODO: if this fails, wouldn't tokenizer be in an undefined state, from the parser perspective?
bool RequireToken(Tokenizer* tokenizer, TokenType desired_type) {
  Token token = GetToken(tokenizer);
  bool result =  token.type == desired_type;
  return result;
}


struct Node {
  char* name;
  u8 id;
  u8 clock_prio;
  char* ip_0;
  char* ip_1;
  char* port_loc;
  char* port_lan;
};

typedef char** StringList;

struct App {
  char* name;
  StringList nodes;
  char* state;
};

void AllocStringField(char** dest, Token* token, StackAllocator* stack) {
  *dest = (char*) stack->Alloc(token->len - 1);
  memcpy(*dest, token->text + 1, token->len - 2);
  (*dest)[token->len - 2] = NULL;
}

void AllocIdentifierField(char** dest, Token* token, StackAllocator* stack) {
  *dest = (char*) stack->Alloc(token->len + 1);
  memcpy(*dest, token->text, token->len);
  (*dest)[token->len] = NULL;
}

void ParseAllocListOfStrings(StringList* lst, Tokenizer* tokenizer, StackAllocator* stack) {
  char* start = tokenizer->at;

  u32 list_len = 0;
  bool counting = true;
  while (counting) {
    Token token = GetToken(tokenizer);

    switch (token.type)
    {
      case TOK_COMMA: {
      } break;

      case TOK_IDENTIFIER: {
        ++list_len;
      } break;

      case TOK_STRING: {
        ++list_len;
      } break;

      case TOK_NUMERIC: {
        assert( 1 == 0 );
      } break;

      case TOK_DOT: {
        // TODO: extend the tokenizer - this could be a float 
        assert( 1 == 0 );
      } break;

      default: {
        counting = false;
      } break;
    }
  }

  // now we can assign list length
  *lst = (StringList) stack->Alloc(list_len * sizeof(StringList));

  // reset 
  u32 idx = 0;
  tokenizer->at = start;
  while (idx < list_len) {
    Token token = GetToken(tokenizer);

    switch (token.type)
    {
      case TOK_COMMA: {
      } break;

      case TOK_IDENTIFIER: {
        AllocIdentifierField(&(*lst)[idx], &token, stack);

        ++idx;
      } break;

      case TOK_STRING: {
        AllocStringField(&(*lst)[idx], &token, stack);

        ++idx;
      } break;

      default: {
        counting = false;
      } break;
    }
  }
}

Node ParseNode(Tokenizer* tokenizer, StackAllocator* stack) {
  Token token;
  Node node = {};

  u8 fields_parsed = 0;
  bool pcheck = false;
  bool parsing = true;
  while (fields_parsed < 7) {
    token = GetToken(tokenizer);

    if (TokenEquals(&token, "Name")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&node.name, &token, stack);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "ID")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      // NOTE: assuming the format 0xab for hex number ab ..
      token = GetToken(tokenizer);
      node.id = strtol(token.text, NULL, 16);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "clock_prio")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      node.clock_prio = (u8) atoi(token.text);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "network_address_0")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&node.ip_0, &token, stack);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "network_address_1")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&node.ip_1, &token, stack);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "port_loc")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&node.port_loc, &token, stack);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "port_lan")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
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
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&app.name, &token, stack);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "Node")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);
      pcheck = RequireToken(tokenizer, TOK_LSBRACK);
      assert(pcheck == true);

      ParseAllocListOfStrings(&app.nodes, tokenizer, stack);

      pcheck = RequireToken(tokenizer, TOK_RSBRACK);
      assert(pcheck == true);

      ++fields_parsed;
    } else 
    if (TokenEquals(&token, "State")) {
      pcheck = RequireToken(tokenizer, TOK_COLON);
      assert(pcheck == true);
      pcheck = RequireToken(tokenizer, TOK_LSBRACK);
      assert(pcheck == true);

      token = GetToken(tokenizer);
      AllocStringField(&app.state, &token, stack);

      pcheck = RequireToken(tokenizer, TOK_RSBRACK);
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
  char* text = LoadFile("config.yaml", true);
  if (text == NULL) {
    printf("could not load file");
    exit(1);
  }

  Tokenizer tokenizer = {};
  ParseMode parsing_mode;

  // read the number of nodes / apps present
  u8 num_nodes = 0;
  u8 num_apps = 0;

  tokenizer.at = text;
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
        if (TokenEquals(&token, "NODE") && RequireToken(&tokenizer, TOK_COLON)) {
          parsing_mode = NODES;
        } else 
        if (TokenEquals(&token, "APP") && RequireToken(&tokenizer, TOK_COLON)) {
          parsing_mode = APPS;
        }
      } break;

      default: {
      } break;
    }
  }

  StackAllocator stack(SIXTEEN_K);
  ArrayList nodes(stack.Alloc(sizeof(Node) * num_nodes), sizeof(Node));
  ArrayList apps(stack.Alloc(sizeof(App) * num_apps), sizeof(App));

  tokenizer.at = text;
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
        if (TokenEquals(&token, "NODE") && RequireToken(&tokenizer, TOK_COLON)) {
          parsing_mode = NODES;
        } else 
        if (TokenEquals(&token, "APP") && RequireToken(&tokenizer, TOK_COLON)) {
          parsing_mode = APPS;
        }
      } break;

      default: {
      } break;
    }
  }
}


void TestTokenizer() {
  char* filename = (char*) "token.h";
  char* text = LoadFile(filename, true);
  if (text == NULL) {
    printf("could not load file");
    exit(1);
  }

  Tokenizer tokenizer = {};
  tokenizer.at = text;

  u32 DB_idx = 0;

  bool parsing = true;
  while (parsing) {
    Token token = GetToken(&tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        printf("%d TOK_ENDOFSTREAM\n", DB_idx);

        parsing = false;
      } break;

      case TOK_UNKNOWN: {
        printf("%d TOK_UNKNOWN: %.1s\n", DB_idx, token.text);

        // skip this token
      } break;

      case TOK_IDENTIFIER: {
        printf("%d TOK_IDENTIFIER: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_NUMERIC: {
        //printf("%d TOK_NUMERIC: %.*s\n", db_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_STRING: {
        printf("%d TOK_STRING: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      case TOK_CHAR: {
        printf("%d TOK_CHAR: %.*s\n", DB_idx, token.len, token.text);

        // action code here
      } break;

      // etc. ...
    }

    ++DB_idx;
  }
}


#endif