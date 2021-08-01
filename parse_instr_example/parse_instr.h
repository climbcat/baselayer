#ifndef __PARSE_INSTR_H__
#define __PARSE_INSTR_H__

//
// Written by Jakob Garde, july 2021.
//

#include <cstdint>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>


//
// type utils


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define KILOBYTE 1024
#define MEGABYTE 1024 * 1024
#define SIXTEEN_M 16 * 1024 * 1024

#define SIXTEEN_K 16 * 1024
#define THIRTYTWO_K 32 * 1024
#define SIXTYFOUR_K 64 * 1024


//
// memory utils


class StackAllocator {
public:
  void* root = NULL;
  u32 max_size = 0;
  u32 num_blocks = 0;
  u32 used = 0;
  bool locked = false;

  StackAllocator(u32 total_size) {
    this->max_size = total_size;
    this->used = 0;
    this->root = malloc(this->max_size);
  }

  ~StackAllocator() {
    free(this->root);
  }

  void Clear() {
    this->used = 0;
    this->num_blocks = 0;
  }

  void* Alloc(u32 size) {
    assert(this->locked == false);

    if (size > this->max_size - this->used)
      return NULL;

    void* retval = (u8*) this->root + this->used;
    this->used += size;
    this->num_blocks++;
    return retval;
  }

  void* AllocOpenEnded() {
    assert(this->locked == false);

    this->locked = true;
    return (u8*) this->root + this->used;
  }

  void CloseOpenEnded(u32 final_size) {
    assert(this->locked == true);

    this->locked == false;
    this->Alloc(final_size);
  }

  bool Free(void* addr) {
    u32 relative_location = (u8*) addr - (u8*) this->root;
    assert(relative_location >= 0);
    assert(relative_location < this->max_size);

    if (relative_location >= this->used)
      return false;

    memset((u8*) this->root + relative_location, this->used - relative_location, 0);

    this->used = relative_location;
    this->num_blocks--;
    return true;
  }
};


inline void ArrayPut(void* lst, u32 element_size, u32 array_len, u32 at_idx, void* item) {
  assert(at_idx <= array_len);

  u8* dest = (u8*) lst + at_idx * element_size;
  memcpy(dest, item, element_size);
}
inline void ArrayShift(void* lst, int element_size, u32 array_len, int at_idx, int offset) {
  // TODO: go back to using u32! (args were converted to int to enable arithmetics)
  assert(at_idx >= 0);

  u8* src = (u8*) lst + at_idx * element_size;
  u8* dest = (u8*) lst + (at_idx + offset) * element_size;
  if (at_idx + offset < 0) {
    dest = (u8*) lst;
    src = (u8*) lst + abs(offset) * element_size;
  }

  assert(src - (u8*) lst > 0);
  assert(dest - (u8*) lst >= 0);
  assert(src - (u8*) lst <= array_len * element_size);
  assert(dest - (u8*) lst < array_len * element_size);
  assert(array_len - at_idx > 0);

  memmove(dest, src, (array_len - at_idx) * element_size);
}


template<typename T>
struct ArrayListT {
  T* lst = NULL;
  u32 element_size = 0;
  u32 len = 0;
  void Init(void* memloc) {
    this->lst = (T*) memloc;
    this->element_size = sizeof(T);
  }
  void Add(T* item) {
    assert(element_size != 0);
    this->len++;
    ArrayPut(this->lst, this->element_size, this->len, this->len - 1, item);
  }
  void Insert(T* item, u32 at_idx) {
    assert(element_size != 0);
    ArrayShift(this->lst, this->element_size, this->len, at_idx, 1);
    this->len++;
    ArrayPut(this->lst, this->element_size, this->len, at_idx, item);
  }
  void Remove(u32 at_idx) {
    assert(element_size != 0);
    ArrayShift(this->lst, this->element_size, this->len, at_idx, -1);
    this->len--;
  }
  T* At(u32 idx) {
    assert(element_size != 0);
    return this->lst + idx;
  }
};


//
// various utils


char* LoadFilePath(char* filepath, StackAllocator *stack = NULL) {
  char * buffer = NULL;
  long length;
  FILE * f = fopen(filepath, "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    if (stack != NULL) {
      buffer = (char*) stack->Alloc(length);
    }
    else {
      buffer = (char*) malloc(length);
    }
    if (buffer)
      fread (buffer, 1, length, f);
    fclose (f);
  }

  return buffer;
}

char* LoadFile(char* filename, bool use_cwd = true, StackAllocator *stack = NULL) {
  if (use_cwd) {
    char cwd[KILOBYTE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      strcat(cwd, "/");
      strcat(cwd, filename);
      filename = (char*) cwd;

      return LoadFilePath(filename, stack);
    }
    return NULL;
  }
  else
    return LoadFilePath(filename, stack);
}

// TODO: get a portable version
ArrayListT<char*> GetFilesInFolderPaths(char *rootpath, StackAllocator *stack) {
  ArrayListT<char*> result;
  struct dirent *dir;
  u32 count = 0;
  u32 rootpath_len = strlen(rootpath);

  DIR *d = opendir(rootpath);
  if (d) {

    while ((dir = readdir(d)) != NULL) {
      ++count;
    }
    closedir(d);
  }

  d = opendir(rootpath);
  if (d) {
    result.Init(stack->Alloc(sizeof(char*) * count));

    d = opendir(rootpath);
    while ((dir = readdir(d)) != NULL) {
      bool needslash = rootpath[rootpath_len-1] != '/';

      u32 len = rootpath_len + int(needslash) + strlen(dir->d_name) + 1;
      char* path = (char*) stack->Alloc( len );

      strcpy(path, rootpath);
      if (needslash) {
        strcat(path, "/");
      }
      strcat(path, dir->d_name);

      result.Add(&path);
    }
    closedir(d);
  }

  return result;
}


//
// tokenizer


#include <cstdlib>
#include <cstring>
#include <cstdio>


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
  void PrintValue(bool newline = true) {
  printf("%.*s", len, text);
  if (newline) {
    printf("\n");
  }
}

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

bool TokenEquals(Token* token, const char* match, bool token_to_upper = false) {
  char* at = (char*) match;
  if (token->len != strlen(match)) {
    return false;
  }
  char ctok;
  char cmatch;
  for (int idx = 0; idx < token->len; idx++, at++) {
    ctok = token->text[idx];
    cmatch = *at;
    if (token_to_upper) {
      ctok = toupper(ctok);
      cmatch = toupper(cmatch);
    }
    if (ctok != cmatch) {
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

Token GetToken(Tokenizer *tokenizer);

TokenType LookAheadNextTokenType(Tokenizer *tokenizer) {
  Tokenizer save = *tokenizer;
  Token token = GetToken(tokenizer);
  *tokenizer = save;
  return token.type;
}

void ParseNumeric(Tokenizer *tokenizer, Token *token) {
  token->type = TOK_INT;

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
      if (token->type != TOK_INT) {
        break;
      }
      token->type = TOK_FLOAT;
    }
    else if ((tokenizer->at[0] == 'e' || tokenizer->at[0] == 'E')
        && tokenizer->at[1] != 0
        && IsNumeric(tokenizer->at[1]))
    {
      if (token->type != TOK_INT && token->type != TOK_FLOAT)
        break;
      token->type = TOK_SCI;
    }

    ++tokenizer->at;
  }
  token->len = tokenizer->at - token->text;
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

    case '-' : {
      TokenType next_type = LookAheadNextTokenType(tokenizer);
      if (next_type == TOK_INT || next_type == TOK_FLOAT ||next_type == TOK_SCI) {
        ParseNumeric(tokenizer, &token);
      }
      else {
        token.type = TOK_DASH;
      }
    } break;
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
        ParseNumeric(tokenizer, &token);
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
    if (desired_value != NULL && !TokenEquals(&token, desired_value)) {
      result = 0;
    }
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
    result = result && TokenEquals(&token, desired_value, true);
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

void AllocString(char** dest, const char *string, StackAllocator* stack) {
  *dest = (char*) stack->Alloc(strlen(string) + 1);
  strcpy(*dest, string);
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

u32 CountTokenSeparatedStuff(char *text, TokenType tok_separator, TokenType tok_exit = TOK_ENDOFSTREAM, TokenType tok_enter = TOK_UNKNOWN, TokenType tok_separator_alt = TOK_UNKNOWN) {
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
    else if (token.type == tok_separator_alt && TOK_UNKNOWN != tok_separator_alt) {
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

bool ParseExpression(Tokenizer *tokenizer, Token *token, const char* end_identifier = NULL) {
  bool result = false;
  u32 bracket_level = 0;
  u32 brace_level = 0;
  u32 sbrack_level = 0;

  EatWhiteSpacesAndComments(tokenizer);
  token->text = tokenizer->at;

  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);
    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_LBRACK: {
        ++bracket_level;
      } break;

      case TOK_RBRACK: {
        if (bracket_level > 0) {
          --bracket_level;
        }
        else {
          parsing = false;
        }
      } break;

      case TOK_LBRACE: {
        ++brace_level;
      } break;

      case TOK_RBRACE: {
        if (brace_level > 0) {
          --brace_level;
        }
      } break;

      case TOK_LSBRACK: {
        ++sbrack_level;
      } break;

      case TOK_RSBRACK: {
        if (sbrack_level > 0) {
          --sbrack_level;
        }
      } break;

      case TOK_SEMICOLON: {
        if (bracket_level == 0 && brace_level == 0 && sbrack_level == 0) {
          parsing = false;
        }
      } break;

      case TOK_COMMA: {
        if (bracket_level == 0 && brace_level == 0 && sbrack_level == 0) {
          parsing = false;
        }
      } break;

      case TOK_IDENTIFIER: {
        result = true;

        if (end_identifier != NULL && TokenEquals(&token, end_identifier)) {
          tokenizer->at -= token.len;
          parsing = false;
        }
      } break;

      default: {
        result = true;
      } break;
    }
  }

  // exit conditions are: EOS, comma, semicolon or rbracket, all of which are 1 char long
  --tokenizer->at;

  token->len = tokenizer->at - token->text;
  token->len = RTrimText(token->text, token->len);
  return result;
}

u32 CountCommaSeparatedSequenceOfExpresions(char *text) {
  Token token;
  Tokenizer tokenizer;
  tokenizer.Init(text);

  u32 count = 0;
  bool flag = false;
  bool parsing = true;
  while (parsing) {
    if (ParseExpression(&tokenizer, &token)) {
      ++count;
    }

    if (!RequireToken(&tokenizer, &token, TOK_COMMA, NULL, false)) {
      parsing = false;
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


//
// mcstas instr parser


struct StructMember {
  char *type = NULL;
  char *name = NULL;
  char *defval = NULL;
};

ArrayListT<StructMember> ParseStructMembers(Tokenizer *tokenizer, StackAllocator *stack) {
  ArrayListT<StructMember> lst;
  u32 count = CountTokenSeparatedStuff(tokenizer->at, TOK_SEMICOLON, TOK_ENDOFSTREAM, TOK_UNKNOWN, TOK_COMMA);
  lst.Init(stack->Alloc(sizeof(StructMember) * count));

  StructMember member;
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);
    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_RPERBRACE: {
        parsing = false;
        tokenizer->at -= token.len;
      } break;

      case TOK_SEMICOLON: {
        lst.Add(&member);
        member = {};
      } break;

      case TOK_COMMA: {
        lst.Add(&member);
        member.name = NULL;
        member.defval = NULL;
      } break;

      case TOK_ASSIGN: {
        if (member.type != NULL && member.name != NULL) {
          if (ParseExpression(tokenizer, &token)) {
            AllocTokenValue(&member.defval, &token, stack);
          }
          else {
            PrintLineError(tokenizer, &token, "Expected value");
            exit(1);
          }
        }
        else {
          PrintLineError(tokenizer, &token, "Unexpected assign:");
          exit(1);
        }
      } break;

      case TOK_IDENTIFIER: {
        if (member.type != NULL && member.name != NULL) {
          PrintLineError(tokenizer, &token, "Expected assign, comma or semicolon");
          exit(1);
        }
        if (member.type != NULL && member.name == NULL) {
          AllocTokenValue(&member.name, &token, stack);
        }
        else if (member.type == NULL && member.name == NULL) {
          AllocTokenValue(&member.type, &token, stack);
        }
      } break;

      default: {
      } break;
    }
  }

  return lst;
}


//
// parsing mcstas .instr files:


struct InstrParam {
  char* name = NULL;
  char* type = NULL;
  char* defaultval = NULL;
};

struct DeclareDef {
  char *text = NULL;
  ArrayListT<StructMember> decls;
};

struct UservarsDef {
  char *text = NULL;
  ArrayListT<StructMember> decls;
};

struct InitializeDef {
  char *text = NULL;
};

struct FinalizeDef {
  char *text = NULL;
};

struct CompParam {
  char *name = NULL;
  char *value = NULL;
};

struct Vector3Strings {
  char *x;
  char *y;
  char *z;
};

struct CompDecl {
  char *type = NULL;
  char *name = NULL;
  char *split = NULL;
  char *extend = NULL;
  char *group = NULL;
  char *copy_name = NULL;
  char *copy_type = NULL;
  char *jump = NULL;
  char *iterate = NULL;
  char *when = NULL;
  bool removable = false;
  ArrayListT<CompParam> params;
  Vector3Strings at;
  Vector3Strings rot;
  char *at_relative = NULL; // value can be ABSOLUTE or a comp name
  char *rot_relative = NULL; // value must be a comp name
  char *percent_include = NULL; // NOTE: includes aren't components
};

struct TraceDef {
  char *text = NULL;
  ArrayListT<CompDecl> comps;
};

struct InstrDef {
  char *name = NULL;
  ArrayListT<InstrParam> params;
  char *dependency;
  DeclareDef declare;
  UservarsDef uservars;
  InitializeDef init;
  TraceDef trace;
  FinalizeDef finalize;
};

ArrayListT<InstrParam> ParseInstrParams(Tokenizer *tokenizer, StackAllocator *stack) {
  u32 count = CountCommaSeparatedSequenceOfExpresions(tokenizer->at);
  ArrayListT<InstrParam> lst;
  lst.Init(stack->Alloc(sizeof(InstrParam) * count));

  Token token;
  for (int i = 0; i < count; i++) {
    InstrParam param = {};

    // name and optional type
    Token tok_one = {};
    Token tok_two = {};
    if (!RequireToken(tokenizer, &tok_one, TOK_IDENTIFIER)) exit(1);
    if (RequireToken(tokenizer, &tok_two, TOK_IDENTIFIER, NULL, false)) {
      AllocTokenValue(&param.type, &tok_one, stack);
      AllocTokenValue(&param.name, &tok_two, stack);
    }
    else {
      AllocTokenValue(&param.name, &tok_one, stack);
    }

    // optional default value
    if (RequireToken(tokenizer, &tok_two, TOK_ASSIGN, NULL, false)) {
      if (!ParseExpression(tokenizer, &token)) {
        PrintLineError(tokenizer, &token, "Expected param value");
        exit(1);
      }
      else {
        AllocTokenValue(&param.defaultval, &token, stack);
      }
    }

    // eat any comma
    RequireToken(tokenizer, &token, TOK_COMMA, NULL, false);
  }


  return lst;
}

char* CopyBracketedTextBlock(Tokenizer *tokenizer, TokenType type_start, TokenType type_end, bool restore_tokenizer, StackAllocator *stack) {
  Tokenizer save = *tokenizer;

  if (type_start != TOK_UNKNOWN) {
    if (!RequireToken(tokenizer, NULL, type_start)) exit(1);
  }

  char *text_start = tokenizer->at;
  LTrim(&text_start);

  Token token = {};
  token.type = TOK_UNKNOWN;
  char *text = NULL;

  u32 dist = LookAheadLenUntilToken(tokenizer, type_end);
  if (dist == 0 && LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
    PrintLineError(tokenizer, &token, "End of file reached");
    exit(1);
  }
  else if (dist == 1) {
    token = GetToken(tokenizer);
  }
  else if (dist > 1) {
    while (token.type != type_end) {
      token = GetToken(tokenizer);
    }

    u32 len_untrimmed = tokenizer->at - text_start - token.len;
    u32 len = RTrimText(text_start, len_untrimmed);

    text = (char*) stack->Alloc(len + 1);
    strncpy(text, text_start, len);
    text[len] = '\0';
  }

  if (restore_tokenizer == true) {
    *tokenizer = save;
  }
  return text;
}

ArrayListT<CompParam> ParseCompParams(Tokenizer *tokenizer, StackAllocator *stack) {
  u32 count = CountCommaSeparatedSequenceOfExpresions(tokenizer->at);

  ArrayListT<CompParam> lst;
  lst.Init(stack->Alloc(sizeof(CompParam) * count));
  Token token;

  for (int i = 0; i < count; i++) {
    CompParam par;

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
    AllocTokenValue(&par.name, &token, stack);

    if (!RequireToken(tokenizer, &token, TOK_ASSIGN)) exit(1);

    if (!ParseExpression(tokenizer, &token)) {
      PrintLineError(tokenizer, &token, "Expected param value.");
      exit(1);
    }
    else {
      AllocTokenValue(&par.value, &token, stack);
      lst.Add(&par);
    }

    if (LookAheadNextToken(tokenizer, TOK_COMMA)) {
      GetToken(tokenizer);
    }
  }

  return lst;
}

char* CopyAllocCharsUntillTok(Tokenizer *tokenizer, TokenType token_type, StackAllocator *stack) {
  char* result;

  EatWhiteSpacesAndComments(tokenizer);

  u32 len_raw = LookAheadLenUntilToken(tokenizer, token_type);
  u32 len = RTrimText(tokenizer->at, len_raw);

  if (len == 1) { // safeguard against the value = zero case
    u8 alen = 3;
    result = (char*) stack->Alloc(alen + 1);
    result[0] = *tokenizer->at;
    result[1] = '.';
    result[2] = '0';
    result[3] = '\0';
  }
  else {
    result = (char*) stack->Alloc(len + 1);
    strncpy(result, tokenizer->at, len);
    result[len] = '\0';
  }

  IncTokenizerUntilAtToken(tokenizer, token_type);
  return result;
}

Vector3Strings ParseVector3(Tokenizer *tokenizer, StackAllocator *stack) {
  Token token;
  Vector3Strings vect;

  if (ParseExpression(tokenizer, &token)) {
    AllocTokenValue(&vect.x, &token, stack);
  }
  else {
    PrintLineError(tokenizer, &token, "Expected value");
    exit(1);
  }
  if (!RequireToken(tokenizer, &token, TOK_COMMA)) exit(1);

  if (ParseExpression(tokenizer, &token)) {
    AllocTokenValue(&vect.y, &token, stack);
  }
  else {
    PrintLineError(tokenizer, &token, "Expected value");
    exit(1);
  }
  if (!RequireToken(tokenizer, &token, TOK_COMMA)) exit(1);

  if (ParseExpression(tokenizer, &token)) {
    AllocTokenValue(&vect.z, &token, stack);
  }
  else {
    PrintLineError(tokenizer, &token, "Expected value");
    exit(1);
  }
  if (!RequireToken(tokenizer, &token, TOK_RBRACK)) exit(1);

  return vect;
}

char* ParseAbsoluteRelative(Tokenizer *tokenizer, StackAllocator *stack) {
  char *result = NULL;
  Token token;
  if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);

  if (TokenEquals(&token, "ABSOLUTE", true)) {
    AllocTokenValue(&result, &token, stack);
  }
  else if (TokenEquals(&token, "RELATIVE", true)) {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
    AllocTokenValue(&result, &token, stack);
  }
  else {
    PrintLineError(tokenizer, &token, "Expected RELATIVE [compname/PREVIOUS] or ABSOLUTE.");
    exit(1);
  }
  return result;
}

// TODO: any way to re-use the generic ParseExpresion?
bool ParseExpression_McStasEndConditions(Tokenizer *tokenizer, Token *token) {
  bool result = false;
  u32 bracket_level = 0;
  u32 brace_level = 0;
  u32 sbrack_level = 0;

  EatWhiteSpacesAndComments(tokenizer);
  token->text = tokenizer->at;

  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);
    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_LBRACK: {
        ++bracket_level;
      } break;

      case TOK_RBRACK: {
        if (bracket_level > 0) {
          --bracket_level;
        }
        else {
          parsing = false;
        }
      } break;

      case TOK_LBRACE: {
        ++brace_level;
      } break;

      case TOK_RBRACE: {
        if (brace_level > 0) {
          --brace_level;
        }
      } break;

      case TOK_LSBRACK: {
        ++sbrack_level;
      } break;

      case TOK_RSBRACK: {
        if (sbrack_level > 0) {
          --sbrack_level;
        }
      } break;

      case TOK_SEMICOLON: {
        if (bracket_level == 0 && brace_level == 0 && sbrack_level == 0) {
          parsing = false;
        }
      } break;

      case TOK_COMMA: {
        if (bracket_level == 0 && brace_level == 0 && sbrack_level == 0) {
          parsing = false;
        }
      } break;

      case TOK_IDENTIFIER: {
        result = true;

        if (TokenEquals(&token, "AT", true)
            || TokenEquals(&token, "ROTATED", true)
            || TokenEquals(&token, "GROUP", true)
            || TokenEquals(&token, "JUMP", true)
            || TokenEquals(&token, "WHEN", true)
            || TokenEquals(&token, "COMPONENT", true)
            || TokenEquals(&token, "END", true)
            || TokenEquals(&token, "EXTEND", true)) {
          tokenizer->at -= token.len;
          parsing = false;
        }
      } break;

      default: {
        result = true;
      } break;
    }
  }

  // exit conditions are: EOS, comma, semicolon or rbracket, all of which are 1 char long
  --tokenizer->at;

  token->len = tokenizer->at - token->text;
  token->len = RTrimText(token->text, token->len);
  return result;
}

ArrayListT<CompDecl> ParseTraceComps(Tokenizer *tokenizer, StackAllocator *stack) {
  ArrayListT<CompDecl> result;
  Tokenizer save = *tokenizer;
  Token token = {};

  // TODO: use the ParseExpression -derived counter instead

  // get number of comps
  u32 count = 0;
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_IDENTIFIER: {
        if (TokenEquals(&token, "COMPONENT", true)) {
          ++count;
        }
        else if (TokenEquals(&token, "FINALIZE", true)) {
          parsing = false;
        }
        else if (TokenEquals(&token, "END", true)) {
          parsing = false;
        }
      } break;

      default: {
      } break;
    }
  }

  // prepare
  *tokenizer = save;
  ArrayListT<CompDecl> lst;
  lst.Init(stack->Alloc(sizeof(CompDecl) * count));

  // parse comps
  for (int idx = 0; idx < count; ++idx) {
    if (!LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      CompDecl comp = {};

      // include [OPTIONAL]
      if (RequireToken(tokenizer, &token, TOK_PERCENT, NULL, false)) {
        if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "include")) exit(1);
        if (!RequireToken(tokenizer, &token, TOK_STRING)) exit(1);
        AllocTokenValue(&comp.percent_include, &token, stack);
        continue;
      }

      // split [OPTIONAL]
      if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "SPLIT", false)) {
        if (ParseExpression_McStasEndConditions(tokenizer, &token)) {
          AllocIdentifierField(&comp.split, &token, stack);
        }
        else {
          AllocString(&comp.split, "1", stack);
        }
      }

      // removable [OPTIONAL]
      if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "REMOVABLE", false)) {
        comp.removable = true;
      }

      // declaration
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "COMPONENT")) exit(1);
      // name
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
      // copy name [OPTIONAL]
      if (TokenEquals(&token, "COPY", true)) {
        if (!RequireToken(tokenizer, &token, TOK_LBRACK)) exit(1);
        if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
        // TODO: whenever there is a copy, the type must be set by a post-processing step
        AllocTokenValue(&comp.copy_name, &token, stack);
        if (!RequireToken(tokenizer, &token, TOK_RBRACK)) exit(1);
      }
      else {
        AllocTokenValue(&comp.name, &token, stack);
      }
      if (!RequireToken(tokenizer, &token, TOK_ASSIGN)) exit(1);
      // type
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
      // copy type [OPTIONAL]
      if (TokenEquals(&token, "COPY", true)) {
        if (!RequireToken(tokenizer, &token, TOK_LBRACK)) exit(1);
        if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
        // TODO: whenever there is a copy, the type must be set by a post-processing step
        AllocTokenValue(&comp.copy_type, &token, stack);
        if (!RequireToken(tokenizer, &token, TOK_RBRACK)) exit(1);
      }
      else {
        AllocTokenValue(&comp.type, &token, stack);
      }

      // params
      if (RequireToken(tokenizer, &token, TOK_LBRACK, NULL, false)) {
        comp.params = ParseCompParams(tokenizer, stack);
        if (!RequireToken(tokenizer, &token, TOK_RBRACK)) exit(1);
      }

      // keywords AT ROTATED JUMP WHEN GROUP EXTEND can occur in any order, and only AT is required
      bool found_AT = false;
      bool parsing_keywords = true;
      while (parsing_keywords) {

        // quick loop exit
        if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "COMPONENT")) {
          parsing_keywords = false;
          continue;
        }

        // location / AT
        else if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "AT", false)) {
          found_AT = true;

          RequireToken(tokenizer, &token, TOK_LBRACK, NULL, false);
          comp.at = ParseVector3(tokenizer, stack);
          RequireToken(tokenizer, &token, TOK_RBRACK, NULL, false);
          comp.at_relative = ParseAbsoluteRelative(tokenizer, stack);
          continue;
        }

        // rotation / ROTATED [OPTIONAL]
        else if (RequireToken(tokenizer, NULL, TOK_IDENTIFIER, "ROTATED", false)) {
          RequireToken(tokenizer, &token, TOK_LBRACK, NULL, false);

          comp.rot = ParseVector3(tokenizer, stack);
          RequireToken(tokenizer, &token, TOK_RBRACK, NULL, false);
          comp.rot_relative = ParseAbsoluteRelative(tokenizer, stack);
          continue;
        }

        // JUMP [OPTIONAL]
        else if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "JUMP", false)) {
          if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
          AllocTokenValue(&comp.jump, &token, stack);
          if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "ITERATE", false)) {
            ParseExpression_McStasEndConditions(tokenizer, &token);
            AllocTokenValue(&comp.iterate, &token, stack);
          }
          continue;
        }

        // WHEN - NOTE: sometimes, the when block is not bracket, thus the added complexity
        else if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "WHEN", false)) {
          RequireToken(tokenizer, &token, TOK_LBRACK, NULL, false);
          if (!ParseExpression_McStasEndConditions(tokenizer, &token)) {
            PrintLineError(tokenizer, &token, "Expected value expression");
            exit(1);
          }
          AllocTokenValue(&comp.when, &token, stack);
          RequireToken(tokenizer, &token, TOK_RBRACK, NULL, false);
          continue;
        }

        // group [OPTIONAL]
        else if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "GROUP", false)) {
          if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
          AllocTokenValue(&comp.group, &token, stack);
          continue;
        }

        // extend [OPTIONAL]
        else if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "EXTEND", false)) {
          comp.extend = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
          continue;
        }

        // default: exit the keyword loop
        else {
          parsing_keywords = false;
        }
      }

      if (!found_AT) {
        // TODO: elaborate this error message
        printf("Component declarations must include the AT definition.\n");
        exit(1);
      }

      lst.Add(&comp);
    }

    else {
      break;
    }
  }

  return lst;
}

InstrDef ParseInstrument(Tokenizer *tokenizer, StackAllocator *stack) {
  Token token;

  if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "DEFINE")) exit(1);
  if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "INSTRUMENT")) exit(1);
  if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);

  // instr header
  InstrDef instr;
  {
    AllocTokenValue(&instr.name, &token, stack);
    if (!RequireToken(tokenizer, NULL, TOK_LBRACK)) exit(1);
    instr.params = ParseInstrParams(tokenizer, stack);
    if (!RequireToken(tokenizer, NULL, TOK_RBRACK)) exit(1);
  }

  // dependency [OPTIONAL]
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "DEPENDENCY", false)) {
    if (!RequireToken(tokenizer, &token, TOK_STRING)) exit(1);
    AllocTokenValue(&instr.dependency, &token, stack);
  }

  // declare [OPTIONAL]
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "DECLARE", false)) {
    instr.declare.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
  }

  // uservars [OPTIONAL]
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "USERVARS", false)) {
    instr.uservars.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, true, stack);
    if (!RequireToken(tokenizer, &token, TOK_LPERBRACE)) exit(1);
    instr.uservars.decls = ParseStructMembers(tokenizer, stack);
    if (!RequireToken(tokenizer, &token, TOK_RPERBRACE)) exit(1);
  }

  // initialize [OPTIONAL]
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "INITIALIZE", false)) {
    instr.init.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
  }

  // trace
  {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "TRACE")) exit(1);
    instr.trace.comps = ParseTraceComps(tokenizer, stack);
  }

  // finalize [OPTIONAL]
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "FINALLY", false)) {
    instr.finalize.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
  }

  return instr;
}

void PrintInstrumentParse(InstrDef instr) {
  printf("instr name: %s\n", instr.name);
  printf("instr dependency: %s\n", instr.dependency);
  printf("instr params:\n");
  for (int i = 0; i < instr.params.len; i++) {
    InstrParam* p = instr.params.At(i);
    printf("  %s %s = %s\n", p->type, p->name, p->defaultval);
  }
  printf("\n");

  printf("init text:\n%s\n\n", instr.declare.text);

  printf("uservars members:\n");
  for (int i = 0; i < instr.uservars.decls.len; ++i) {
    StructMember *memb = instr.uservars.decls.At(i);
    printf("  %s %s = %s\n", memb->type, memb->name, memb->defval);
  }
  printf("\n");

  printf("init text:\n%s\n\n", instr.init.text);

  printf("components:\n");
  for (int i = 0; i < instr.trace.comps.len; ++i) {
    CompDecl *comp = instr.trace.comps.At(i);
    printf("\n  type: %s\n", comp->type);
    printf("  copy_name: %s\n", comp->copy_name);
    printf("  copy_type: %s\n", comp->copy_type);
    printf("  name: %s\n", comp->name);
    printf("  split: %s\n", comp->split);
    printf("  removable: %d\n", comp->removable);
    printf("  params:\n");
    auto lstp = comp->params;
    for (int i = 0; i < lstp.len; ++i) {
      CompParam *param = lstp.At(i);
      printf("    %s = %s\n", param->name, param->value);
    }
    printf("  group: %s\n", comp->group);
    printf("  jump: %s\n", comp->jump);
    printf("  iterate: %s\n", comp->iterate);
    printf("  when: %s\n", comp->when);
    printf("  at:      (%s, %s, %s) %s\n", comp->at.x, comp->at.y, comp->at.z, comp->at_relative);
    printf("  rotated: (%s, %s, %s) %s\n", comp->rot.x, comp->rot.y, comp->rot.z, comp->rot_relative);
    printf("  extend:\n%s\n", comp->extend);
  }
  printf("\n");

  printf("finalize text:\n%s\n\n", instr.finalize.text);
}

void TestParseMcStasInstr(int argc, char **argv) {
  StackAllocator stack(MEGABYTE);

  char *filename = (char*) "PSI.instr";
  char *text = LoadFile(filename, true, &stack);
  if (text == NULL) {
      printf("could not load file %s\n", filename);
      exit(1);
  }
  printf("parsing file %s\n\n", filename);


  Tokenizer tokenizer = {};
  tokenizer.Init(text);

  InstrDef instr = ParseInstrument(&tokenizer, &stack);

  PrintInstrumentParse(instr);
}

bool IsInstrFile(char *filename) {
  // check lenth / get expected location
  u32 len = strlen(filename);
  if (len < 7) {
    return false;
  }

  const char *match = ".instr";
  char cf;
  char cm;
  for (int i = 0; i < 6; i++) {
    cf = filename[i + len - 6];
    cm = match[i];
    if (cf != cm) {
      return false;
    }
  }

  return true;
}


#endif
