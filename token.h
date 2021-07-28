#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "various.h"
#include "memory.h"


/**
* Light-weight, multi-purpose Tokenizer code. Implementation roughly follows handmade hero.
*/

enum TokenType {
  TOK_UNKNOWN, // catch-all for things that aren't defined yet

  TOK_LBRACK, // (
  TOK_RBRACK,
  TOK_LBRACE, // {
  TOK_RBRACE,
  TOK_LSBRACK, // [
  TOK_RSBRACK,
  TOK_LEDGE, // <
  TOK_REDGE,
  TOK_POUND, // #
  TOK_ASTERISK,
  TOK_COMMA,
  TOK_DOT,
  TOK_SLASH, // /
  TOK_DASH, // -
  TOK_PLUS,
  TOK_COLON,
  TOK_SEMICOLON,
  TOK_ASSIGN, // =
  TOK_EXCLAMATION,
  TOK_TILDE,
  TOK_OR, // |
  TOK_AND, // &
  TOK_PERCENT,

  TOK_CHAR,
  TOK_STRING,
  TOK_INT, // 123
  TOK_FLOAT, // 24.5748
  TOK_SCI, // 2.4e21 
  TOK_IDENTIFIER,

  TOK_ENDOFSTREAM,
};

const char* TokenTypeToString(TokenType tpe) {
  switch (tpe)
  {
      case TOK_UNKNOWN: return "TOK_UNKNOWN";
      case TOK_LBRACK: return "TOK_LBRACK";
      case TOK_RBRACK: return "TOK_RBRACK";
      case TOK_LBRACE: return "TOK_LBRACE";
      case TOK_RBRACE: return "TOK_RBRACE";
      case TOK_LSBRACK: return "TOK_LSBRACK";
      case TOK_RSBRACK: return "TOK_RSBRACK";
      case TOK_LEDGE: return "TOK_LEDGE";
      case TOK_REDGE: return "TOK_REDGE";
      case TOK_POUND: return "TOK_POUND";
      case TOK_ASTERISK: return "TOK_ASTERISK";
      case TOK_COMMA: return "TOK_COMMA";
      case TOK_DOT: return "TOK_DOT";
      case TOK_SLASH: return "TOK_SLASH";
      case TOK_DASH: return "TOK_DASH";
      case TOK_PLUS: return "TOK_PLUS";
      case TOK_COLON: return "TOK_COLON";
      case TOK_SEMICOLON: return "TOK_SEMICOLON";
      case TOK_ASSIGN: return "TOK_ASSIGN";
      case TOK_EXCLAMATION: return "TOK_EXCLAMATION";
      case TOK_TILDE: return "TOK_TILDE";
      case TOK_OR: return "TOK_OR";
      case TOK_AND: return "TOK_AND";
      case TOK_PERCENT: return "TOK_PERCENT";

      case TOK_CHAR: return "TOK_CHAR";
      case TOK_STRING: return "TOK_STRING";
      case TOK_INT: return "TOK_INT";
      case TOK_FLOAT: return "TOK_FLOAT";
      case TOK_SCI: return "TOK_SCI";
      case TOK_IDENTIFIER: return "TOK_IDENTIFIER";

      case TOK_ENDOFSTREAM: return "TOK_ENDOFSTREAM";

      default: return "ReturnTokenTypeString__default";
  }
}

void TokenTypePrint(TokenType tpe, bool newline = true) {
  printf("%s", TokenTypeToString(tpe));
  if (newline) {
    printf("\n");
  }
}

struct Tokenizer {
  char *at;
  u32 line = 1;
  char *at_linestart;
  void Init(char *text) {
    line = 1;
    at = text;
    at_linestart = text;
  }
  void AtNewLineChar() {
    ++line;
    at_linestart = at + 1;
  }
};

struct Token {
  TokenType type;
  char* text;
  u16 len;
};

inline
bool IsEndOfLine(char c) {
  return
    c == '\n' ||
    c == '\r';
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
  bool isnum = (c >= '0') && (c <= '9');
  return isnum;
}

inline
bool IsNumericSymbol(char c) {
  bool issymb = ((c == '.') || (c == 'e') || (c == 'E') || (c == '-') || (c == '+'));
  return issymb;
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

    if (tokenizer->at[0] == '\n') {
      tokenizer->AtNewLineChar();
      ++tokenizer->at;
      return;
    }

    ++tokenizer->at;
  }
}

void EatCStyleComment(Tokenizer* tokenizer) {
  tokenizer->at += 2; // assuming at = "/* ... */"
  for(;;) {
    if (tokenizer->at[0] == '\0') {
      return;
    }

    if (tokenizer->at[0] == '\n') {
      tokenizer->AtNewLineChar();
    }

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
    if (IsEndOfLine(tokenizer->at[0])) {
      tokenizer->AtNewLineChar();
    }
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

bool TokenEquals(Token* token, const char* match) { 
  char* at = (char*) match;
  for (int idx = 0; idx < token->len; idx++, at++) {
    if (token->text[idx] != *at) {
      return false;
    }
  }
  bool result = (*at == 0);
  return true;
}

inline
void AllocTokenValue(char **dest, Token *token, StackAllocator *stack) {
  *dest = (char*) stack->Alloc(token->len + 1);
  strncpy(*dest, token->text, token->len);
  (*dest)[token->len] = '\0';
}

inline
void AllocTokenValueAssertType(char **dest, Token *token, TokenType type_assert, StackAllocator *stack) {
  assert( token->type == type_assert );
  AllocTokenValue(dest, token, stack);
}

inline
int FindChar(char *text, char c) {
  u32 dist = 0;
  while (true) {
    if (*text == '\0')
      return -1;
    if (c == *text)
      return dist;
    ++dist;
    ++text;
  }
}

inline
u32 DistEndOfLine(char* text) {
  u32 dist = 0;
  while (true) {
    if ('\0' == *text || IsEndOfLine(*text))
      return dist;
    ++dist;
    ++text;
  }
}

void PrintCurrentLine(Tokenizer *tokenizer) {
  printf("%.*s\n", DistEndOfLine(tokenizer->at_linestart), tokenizer->at_linestart);
}

void PrintCurrentLineMark(Tokenizer *tokenizer, u32 padding = 0) {
  u32 mark = tokenizer->at - tokenizer->at_linestart + padding;
  for (u32 i = 0; i < mark; i++) {
    printf(" ");
  }
  printf("^\n");
}

void PrintLineError(Tokenizer *tokenizer, Token *token, const char* errmsg = NULL) {
  //printf("Error on line %d (%s %.*s): ",
  char* msg = (char*) errmsg;
  if (errmsg == NULL) {
    msg = (char*) "Error.";
  }

  printf("%s:\n", msg);
  char slineno[200];
  sprintf(slineno, "%d| ", tokenizer->line);
  printf("%s", slineno);
  PrintCurrentLine(tokenizer);
  PrintCurrentLineMark(tokenizer, strlen(slineno) + 2 - 1);
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

      while (tokenizer->at[0] != '\0' && tokenizer->at[0] != '"' && !IsEndOfLine(tokenizer->at[0])) {
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
  
      while (tokenizer->at[0] != '\0' && tokenizer->at[0] != '\'' && !IsEndOfLine(tokenizer->at[0])) {
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
        token.type = TOK_INT;

        while (true) {
          if (tokenizer->at[0] != 0
              && !IsNumeric(tokenizer->at[0])
              && !IsNumericSymbol(tokenizer->at[0]))
          {
            break;
          }
          else if (tokenizer->at[1] != 0
              && tokenizer->at[2] != 0
              && IsNumericSymbol(tokenizer->at[1])
              && !IsNumeric(tokenizer->at[2]))
          {
            break;
          }

          if (tokenizer->at[0] == '.'
              && tokenizer->at[1] != 0
              && IsNumeric(tokenizer->at[1]))
          {
            if (token.type != TOK_INT)
              break;
            token.type = TOK_FLOAT;
          }
          else if ((tokenizer->at[0] == 'e' || tokenizer->at[0] == 'E')
              && tokenizer->at[1] != 0
              && IsNumeric(tokenizer->at[1]))
          {
            if (token.type != TOK_INT && token.type != TOK_FLOAT)
              break;
            token.type = TOK_SCI;
          }

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

u32 LookAheadTokenCountOR(Tokenizer *tokenizer, TokenType desired_type, TokenType desired_type_or = TOK_ENDOFSTREAM) {
  Tokenizer save = *tokenizer;

  u32 steps = 0;
  Token token;
  while (true) {
    ++steps;
    token = GetToken(tokenizer);
    if (token.type == TOK_ENDOFSTREAM) {
      return 0;
    }
    else if (token.type == desired_type) {
      break;
    }
    else if (token.type == desired_type_or) {
      break;
    }
  }

  *tokenizer = save;
  return steps;
}

void BasicParsingLoopSwitch(Tokenizer *tokenizer) {
  // template loop / switch
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      default: {
      } break;
    }
  }
}

u32 LookAheadTokenCountNOT(Tokenizer *tokenizer, TokenType desired_type, TokenType avoid_type = TOK_ENDOFSTREAM) {
  Tokenizer save = *tokenizer;

  u32 steps = 0;
  Token token;
  while (true) {
    ++steps;
    token = GetToken(tokenizer);
    if (token.type == TOK_ENDOFSTREAM || token.type == avoid_type) {
      return 0;
    }
    if (token.type == desired_type) {
      break;
    }
  }

  *tokenizer = save;
  return steps;
}

inline
u32 LookAheadTokenCount(Tokenizer *tokenizer, TokenType desired_type) {
  return LookAheadTokenCountOR(tokenizer, desired_type);
}

u32 LookAheadTokenTextlen(Tokenizer *tokenizer, TokenType desired_type) {
  Tokenizer save = *tokenizer;

  Token token;
  while (true) {
    token = GetToken(tokenizer);
    if (token.type == TOK_ENDOFSTREAM) {
      return 0;
    }
    else if (token.type == desired_type) {
      break;
    }
  }

  u32 result = tokenizer->at - token.len - save.at;
  *tokenizer = save;
  return result;
}

bool LookAheadNextToken(Tokenizer *tokenizer, TokenType desired_type, const char *desired_value = NULL) {
  Tokenizer save = *tokenizer;
  Token token = GetToken(tokenizer);
  bool result = 0;
  if (token.type == desired_type) {
    result = 1;
  }
  if (desired_value != NULL && !TokenEquals(&token, desired_value)) {
    result = 0;
  }
  *tokenizer = save;
  return result;
}

// will inc tokenizer ONLY IF token requirement is satisfied
bool RequireToken(Tokenizer* tokenizer, Token *token_dest, TokenType desired_type, const char *desired_value = NULL) {
  Tokenizer resume = *tokenizer;

  Token token = GetToken(tokenizer);
  if (token_dest != NULL) {
    *token_dest = token;
  }
  bool result =  token.type == desired_type;
  if (desired_value != NULL) {
    result = result && TokenEquals(&token, desired_value);
  }

  if (result == false) {
    *tokenizer = resume;
    char msg[200];
    if (desired_value == NULL) {
      sprintf(msg, "Expected %s", TokenTypeToString(desired_type));
    }
    else {
      sprintf(msg, "Expected %s", desired_value);
    }
    PrintLineError(tokenizer, &token, msg);
  }
  return result;
}

typedef char** StringList;

void AllocStringField(char** dest, Token* token, StackAllocator* stack) {
  *dest = (char*) stack->Alloc(token->len - 1);
  memcpy(*dest, token->text + 1, token->len - 2);
  (*dest)[token->len - 2] = '\0';
}

void AllocIdentifierField(char** dest, Token* token, StackAllocator* stack) {
  *dest = (char*) stack->Alloc(token->len + 1);
  memcpy(*dest, token->text, token->len);
  (*dest)[token->len] = '\0';
}

void ParseAllocCommaSeparatedListOfStrings(StringList* lst, Tokenizer* tokenizer, StackAllocator* stack) {
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

      case TOK_INT: {
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


#endif