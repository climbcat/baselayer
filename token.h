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

// TODO: expand with mcstas-like symbols - %{, %}, COMPONENT, TRACE, etc...
// TODO: introduce a number catch-ass

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
  TOK_RPERBRACE, // %{
  TOK_LPERBRACE, // %}

  TOK_CHAR,
  TOK_STRING,
  TOK_INT, // 123
  TOK_FLOAT, // 24.5748
  TOK_SCI, // 2.4e21 
  TOK_IDENTIFIER,

  TOK_MCSTAS_END,

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
    case TOK_RPERBRACE: return "TOK_RPERBRACE";
    case TOK_LPERBRACE: return "TOK_LPERBRACE";

    case TOK_CHAR: return "TOK_CHAR";
    case TOK_STRING: return "TOK_STRING";
    case TOK_INT: return "TOK_INT";
    case TOK_FLOAT: return "TOK_FLOAT";
    case TOK_SCI: return "TOK_SCI";
    case TOK_IDENTIFIER: return "TOK_IDENTIFIER";

    case TOK_MCSTAS_END: return "TOK_MCSTAS_END";

    case TOK_ENDOFSTREAM: return "TOK_ENDOFSTREAM";

    default: return "ReturnTokenTypeString__default";
  }
}

const char* TokenTypeToSymbol(TokenType tpe) {
  switch (tpe)
  {
    case TOK_UNKNOWN: return "(unknown)";
    case TOK_LBRACK: return "(";
    case TOK_RBRACK: return ")";
    case TOK_LBRACE: return "{";
    case TOK_RBRACE: return "}";
    case TOK_LSBRACK: return "[";
    case TOK_RSBRACK: return "]";
    case TOK_LEDGE: return "<";
    case TOK_REDGE: return ">";
    case TOK_POUND: return "#";
    case TOK_ASTERISK: return "*";
    case TOK_COMMA: return ",";
    case TOK_DOT: return ".";
    case TOK_SLASH: return "/";
    case TOK_DASH: return "-";
    case TOK_PLUS: return "+";
    case TOK_COLON: return ":";
    case TOK_SEMICOLON: return ";";
    case TOK_ASSIGN: return "=";
    case TOK_EXCLAMATION: return "!";
    case TOK_TILDE: return "~";
    case TOK_OR: return "|";
    case TOK_AND: return "&";
    case TOK_PERCENT: return "%";
    case TOK_RPERBRACE: return "%{";
    case TOK_LPERBRACE: return "%}";

    case TOK_CHAR: return "[char]";
    case TOK_STRING: return "[string]";
    case TOK_INT: return "[integer]";
    case TOK_FLOAT: return "[float]";
    case TOK_SCI: return "[float scientific]";
    case TOK_IDENTIFIER: return "[identifier]";

    case TOK_MCSTAS_END: return "END";

    case TOK_ENDOFSTREAM: return "[eos]";

    default: return "ReturnTokenTypeSymbol__default";
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
    if (tokenizer->at[0] == '\0') {
      return;
    }

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
u32 Trim(char **src, u32 maxlen) { // trim whitespaces, sets *src = start and returns len (without null termination)
  u32 idx = 0;
  char *at = *src;
  while ( IsWhitespace(*at) ) {
    ++at;
    ++idx;
  }

  *src = at;
  while ( !IsWhitespace(*at) && idx < maxlen) {
    ++at;
    ++idx;
  }
  return at - *src;
}

inline
void LTrim(char **src) { // sets *src to first non-whitespace char
  u32 idx = 0;
  char *at = *src;
  while ( IsWhitespace(*at) ) {
    ++at;
    ++idx;
  }
  *src = at;
}

inline
u32 RTrimText(char *src, u32 textlen) { // returns len corresponding to last non-whitespace char
  char *at = src + textlen - 1;
  u32 steps_back = 0;
  while (at > src && IsWhitespace(*at)) {
    --at;
    ++steps_back;
  }
  return textlen - steps_back;
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

void PrintLineError(Tokenizer *tokenizer, Token *token, const char* errmsg = NULL) {
  char* msg = (char*) errmsg;
  if (errmsg == NULL) {
    msg = (char*) "Error.";
  }

  u32 toklen = 0;
  if (token != NULL) {
    toklen = token->len;
  }

  // print message
  printf("%s\n", msg);

  // print line nb. tag
  char lineno_tag[200];
  sprintf(lineno_tag, "%d| ", tokenizer->line);
  printf("%s", lineno_tag);

  // print line
  printf("%.*s\n", DistEndOfLine(tokenizer->at_linestart), tokenizer->at_linestart);

  // print marker
  u32 mark = (tokenizer->at - toklen) - tokenizer->at_linestart + strlen(lineno_tag);
  for (u32 i = 0; i < mark; i++) {
    printf(" ");
  }
  printf("^\n");
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

    case '%' : {
      if (tokenizer->at[0] && tokenizer->at[0] == '{') {
        token.type = TOK_LPERBRACE;
        token.len = 2;
        ++tokenizer->at;
      }
      else if (tokenizer->at[0] && tokenizer->at[0] == '}') {
        token.type = TOK_RPERBRACE;
        token.len = 2;
        ++tokenizer->at;
      }
      else {
        token.type = TOK_PERCENT; break;
      }
    } break;

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

        if (TokenEquals(&token, "END")) {
          token.type = TOK_MCSTAS_END;
        }
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
            if (token.type != TOK_INT) {
              break;
            }
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

inline
u32 LookAheadLenChars(char* at, char until) {
  char *start = at;
  while (*at != until && *at != '\0') {
    ++at;
  }
  return at - start;
}

inline
u32 LookAheadLenEoL(char* at) {
  char *start = at;
  while (*at != '\0' && !IsEndOfLine(*at)) {
    ++at;
  }
  return at - start;
}

inline
u32 LookAheadLenCharsFailAtEolAndEos(char* at, char until) {
  char *start = at;
  while (*at != until && *at != '\0' && !IsEndOfLine(*at)) {
    ++at;
  }
  return at - start;
}

u32 LookAheadLenUntilToken(Tokenizer *tokenizer, TokenType desired_type, bool failat_eol = false) {
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

  u32 result = tokenizer->at - token.len - save.at; // TODO: use rewind feature !!
  u32 result_eol = LookAheadLenEoL(save.at);
  *tokenizer = save;

  if (failat_eol && result_eol < result) {
    return 0;
  }

  return result;
}

bool LookAheadNextToken(Tokenizer *tokenizer, TokenType desired_type, const char *desired_value = NULL) {
  Tokenizer save = *tokenizer;
  Token token = GetToken(tokenizer);
  bool result = 0;

  if (token.type == TOK_ENDOFSTREAM) {
    result = 0;
  }
  else if (token.type == desired_type) {
    result = 1;
  }
  else if (desired_value != NULL && !TokenEquals(&token, desired_value)) {
    result = 0;
  }
  *tokenizer = save;
  return result;
}

void IncTokenizerUntilAtToken(Tokenizer *tokenizer, TokenType until_type) {
  while (true) {
    if (LookAheadNextToken(tokenizer, until_type)) {
      return;
    }
    if (LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      return;
    }
    GetToken(tokenizer);
  }
}

// will inc tokenizer ONLY IF token requirement is satisfied
bool RequireToken(Tokenizer* tokenizer, Token *token_dest, TokenType desired_type, const char *desired_value = NULL, bool print_error = true) {
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
    if (print_error) {
      char msg[200];
      if (desired_value == NULL) {
        sprintf(msg, "Expected %s", TokenTypeToString(desired_type));
      }
      else {
        sprintf(msg, "Expected %s", desired_value);
      }
      PrintLineError(tokenizer, &token, msg);
    }

    *tokenizer = resume;
  }
  return result;
}

typedef char** StringList;

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

u32 MinU(u32 a, u32 b) {
  if (a <= b) {
    return a;
  }
  else {
    return b;
  }
}

u32 MaxU(u32 a, u32 b) {
  if (a >= b) {
    return a;
  }
  else {
    return b;
  }
}

u32 CountNumCharsInText(char *text, char match) {
  u32 count = 0;
  while (*text != '\0') {
    if (*text == match) {
      ++count;
    }
    ++text;
  }
  return count;
}

u32 CountTokenSeparatedStuff(char *text, TokenType tok_separator, TokenType tok_exit = TOK_ENDOFSTREAM, TokenType tok_enter = TOK_UNKNOWN) {
  Tokenizer tokenizer;
  tokenizer.Init(text);

  if (tok_enter != TOK_UNKNOWN) {
    if (!RequireToken(&tokenizer, NULL, tok_enter)) return 0;
  }

  bool flag = true;
  u32 count = 0;
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(&tokenizer);

    if (token.type == TOK_ENDOFSTREAM) {
      parsing = false;
    }
    else if (token.type == tok_separator) {
      flag = true;
    }
    else if (token.type == tok_exit) {
      parsing = false;
    }
    else {
      if (flag == true) {
        ++count;
        flag = false;
      }
    }
  }

  return count;
}

void ParseAllocCommaSeparatedListOfStrings(StringList* lst, Tokenizer* tokenizer, StackAllocator* stack) {
  Tokenizer save = *tokenizer;

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
  *lst = (StringList) stack->Alloc(list_len * sizeof(StringList));

  // reset 
  u32 idx = 0;
  *tokenizer = save;
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