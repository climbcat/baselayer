#ifndef __TOKEN_H__
#define __TOKEN_H__


#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "types.h"
#include "various.h"

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
  tokenizer->at += 2; // assuming at = "// ... \n"
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
        if (tokenizer->at[1] == '/') {
          EatCppStyleComment(tokenizer);
        }
        else if (tokenizer->at[1] == '*') {
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

    else {
      break;
    }
  }
}

bool IsTokenEqualTo(Token* tok, char* word) {
  for (char* at = tok->text; at < tok->text + tok->len; tok++, word++) {
    if (*at != *word)
      return false;
  }
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