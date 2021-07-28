#include "memory.h"
#include "token.h"


//
// parsing C structs


struct StructMember {
  char *type = NULL;
  char *name = NULL;
  char *defval = NULL;
};


ArrayListT<StructMember> ParseStructMembers(char *text, StackAllocator *stack) {
  Tokenizer _tokenizer_var;
  Tokenizer *tokenizer = &_tokenizer_var;
  tokenizer->Init(text);

  // count the number of members by counting semi-colons
  bool parsing = true;
  u32 count = GetNumTokenSeparatedStuff(text, TOK_SEMICOLON);

  ArrayListT<StructMember> lst;
  lst.Init(stack->Alloc(sizeof(StructMember) * count));
  Token token;

  while (true) {
    StructMember member;

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
    AllocTokenValue(&member.type, &token, stack);

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
    AllocTokenValue(&member.name, &token, stack);

    if (LookAheadNextToken(tokenizer, TOK_ASSIGN)) {
      GetToken(tokenizer); // skip the assign
      u32 len = LookAheadTokenTextlen(tokenizer, TOK_SEMICOLON);
      if (len == 0) {
        PrintLineError(tokenizer, &token, "Expected: ;");
        exit(1);
      }
      member.defval = (char*) stack->Alloc(len);
      strncpy(member.defval, tokenizer->at, len);

      tokenizer->at += len;
    }

    if (!RequireToken(tokenizer, &token, TOK_SEMICOLON)) exit(0);

    lst.Add(&member);

    // positive exit condition
    if (LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      break;
    }
  }

  return lst;
}


//
// parsing mcstas .instr files:


struct InstrParam {
  char* name;
  char* type;
  char* defaultval;
};

struct DeclareDef {
  char * text;
};

struct InitializeDef {
  char * text;
};

struct FinalizeDef {
  char * text;
};

struct CompParam {
  char* name;
  char* value;
};

struct Vector3Strings {
  char* x;
  char* y;
  char* z;
};

struct CompDecl {
  char *type = NULL;
  char *name = NULL;
  char *extend = NULL;
  u32 split = 0;
  ArrayListT<CompParam> params;
  Vector3Strings at;
  Vector3Strings rot;
  char* at_relative = NULL; // value can be ABSOLUTE or a comp name
  char* rot_relative = NULL; // value must be a comp name
};

struct TraceDef {
  char * text;
  ArrayListT<CompDecl> comps;
};

struct InstrDef {
  char *name;
  ArrayListT<InstrParam> params;
  DeclareDef declare;
  InitializeDef init;
  TraceDef trace;
  FinalizeDef finalize;
};

ArrayListT<InstrParam> ParseInstrParams(Tokenizer *tokenizer, StackAllocator *stack) {
  u32 count = GetNumTokenSeparatedStuff(tokenizer->at, TOK_COMMA, TOK_RBRACK);

  ArrayListT<InstrParam> alst;
  alst.Init(stack->Alloc(sizeof(InstrParam) * count));

  bool parsing = true;
  while (parsing) {
    if (LookAheadNextToken(tokenizer, TOK_RBRACK)) {
      return alst;
    }
    Token token = GetToken(tokenizer);
    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
        assert(1 == 0);
      } break;

      case TOK_COMMA: {

      } break;

      case TOK_RBRACK: {
        parsing = false;
      } break;

      case TOK_IDENTIFIER: {
        InstrParam param = {};
        u32 count = LookAheadTokenCountOR(tokenizer, TOK_COMMA, TOK_RBRACK);

        if (count == 4) {
          // type varname = defval
          AllocTokenValueAssertType(&param.type, &token, TOK_IDENTIFIER, stack);

          token = GetToken(tokenizer);
          AllocTokenValueAssertType(&param.name, &token, TOK_IDENTIFIER, stack);

          GetToken(tokenizer);

          token = GetToken(tokenizer);
          if (token.type != TOK_FLOAT && token.type != TOK_INT && token.type != TOK_SCI && token.type != TOK_STRING) {
            tokenizer->at -= token.len;
            PrintLineError(tokenizer, &token, "Expected number or string");
            exit(1);
          }
          AllocTokenValue(&param.defaultval, &token, stack);
        }
        else if (count == 3) {
          // parname = defval
          AllocTokenValueAssertType(&param.name, &token, TOK_IDENTIFIER, stack);

          GetToken(tokenizer);

          token = GetToken(tokenizer);
          if (token.type != TOK_FLOAT && token.type != TOK_INT && token.type != TOK_SCI && token.type != TOK_STRING) {
            PrintLineError(tokenizer, &token, "Expected number or string");
            exit(1);
          }
          AllocTokenValue(&param.defaultval, &token, stack);
        }
        else if (count == 2) {
          // type parname
          AllocTokenValueAssertType(&param.type, &token, TOK_IDENTIFIER, stack);

          token = GetToken(tokenizer);
          AllocTokenValueAssertType(&param.name, &token, TOK_IDENTIFIER, stack);
        }
        else if (count == 1) {
          // parname
          AllocTokenValueAssertType(&param.defaultval, &token, TOK_IDENTIFIER, stack);
        }
        else {
          PrintLineError(tokenizer, &token, "Parameter ill defined");
          exit(1);
        }
        alst.Add(&param);
 
      } break;

      default: {
      } break;
    }
  }

  return alst;
}

char* CopyPercentBraceTextBlock(Tokenizer *tokenizer, StackAllocator *stack) {
  Token token;
  if (!RequireToken(tokenizer, &token, TOK_PERCENT)) exit(1);
  if (!RequireToken(tokenizer, &token, TOK_LBRACE)) exit(1);

  Tokenizer save = *tokenizer;
  char *text = NULL;

  while (true) {
    u32 dist = LookAheadTokenTextlen(tokenizer, TOK_PERCENT);
    if (dist == 0 && LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      PrintLineError(tokenizer, &token, "End of file reached");
      exit(1);
    }
    tokenizer->at += dist;
    if (!RequireToken(tokenizer, &token, TOK_PERCENT)) exit(0);
    if (!RequireToken(tokenizer, &token, TOK_RBRACE)) exit(0);

    u32 len = tokenizer->at - save.at - 2;
    text = (char*) stack->Alloc(len + 1);
    strncpy(text, save.at, len);
    text[len] = '\0';

    break;
  }
  return text;
}

char* GetTextUntillEOSOrBlockClose(Tokenizer *tokenizer, StackAllocator *stack) {
  Tokenizer save = *tokenizer;

  // basic parsing pattern:
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
      } break;

      case TOK_PERCENT: {
        if (LookAheadNextToken(tokenizer, TOK_RBRACE)) {
          tokenizer->at -= 1;
          parsing = false;
        }
      } break;

      case TOK_IDENTIFIER: {
        if (TokenEquals(&token, "END")) {
          tokenizer->at -= 3;
          parsing = false;
        }
      } break;

      default: {
      } break;
    }
  }

  u32 len = tokenizer->at - save.at;
  char *result = (char*) stack->Alloc(len + 1);
  strncpy(result, save.at, len);
  result[len] = '\0';
  return result;
}

ArrayListT<CompParam> ParseCompParams(Tokenizer *tokenizer, StackAllocator *stack) {
  CompParam par;
  Token token;



  // find number of pars, check integrity
  Tokenizer save = *tokenizer;
  u32 count = 0;
  bool parsing = true;
  while (parsing) {
    Token token = GetToken(tokenizer);

    switch ( token.type ) {
      case TOK_ENDOFSTREAM: {
        parsing = false;
        PrintLineError(tokenizer, &token, "Unexpected end of stream");
        exit(1);
      } break;

      case TOK_IDENTIFIER: {
        count = MaxU(count, 1);
      } break;

      case TOK_COMMA: {
        ++count;
        count = MaxU(count, 2);
      } break;

      case TOK_RBRACK: {
        parsing = false;
      } break;

      default: {
      } break;
    }
  }

  ArrayListT<CompParam> lst;
  lst.Init(stack->Alloc(sizeof(CompParam) * count));
  *tokenizer = save;

  // parse individual params
  for (int i = 0; i < count; i++) {
    CompParam par;

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
    AllocTokenValue(&par.name, &token, stack);

    if (!RequireToken(tokenizer, &token, TOK_ASSIGN)) exit(0);

    EatWhiteSpacesAndComments(tokenizer);
    u32 vallen = MinU(LookAheadTokenTextlen(tokenizer, TOK_COMMA), LookAheadTokenTextlen(tokenizer, TOK_RBRACK));
    if (vallen == 0) {
      PrintLineError(tokenizer, &token, "Expected param value.");
      exit(1);
    }
    else {
      EatWhiteSpacesAndComments(tokenizer);
      token.text = tokenizer->at;
      token.len = vallen;
      AllocTokenValue(&par.value, &token, stack);
      lst.Add(&par);
      tokenizer->at += vallen;
    }

    if (LookAheadNextToken(tokenizer, TOK_COMMA)) {
      GetToken(tokenizer);
    }
  }

  return lst;
}

u32 Trim(char **src, u32 maxlen) { // trim whitespaces, sets *src = start and returns len (no null term)
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

char* CopyAllocCharsUntillTok(TokenType token_type, Tokenizer *tokenizer, StackAllocator *stack) {
  char* result;
  u32 len = LookAheadTokenTextlen(tokenizer, token_type);
  len = Trim(&tokenizer->at, len);

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

  tokenizer->at += len;
  return result;
}

Vector3Strings ParseVector3(Tokenizer *tokenizer, StackAllocator *stack) {
  Token token;
  Vector3Strings vect;

  EatWhiteSpacesAndComments(tokenizer);
  vect.x = CopyAllocCharsUntillTok(TOK_COMMA, tokenizer, stack);

  RequireToken(tokenizer, &token, TOK_COMMA);

  EatWhiteSpacesAndComments(tokenizer);
  vect.y = CopyAllocCharsUntillTok(TOK_COMMA, tokenizer, stack);

  RequireToken(tokenizer, &token, TOK_COMMA);

  EatWhiteSpacesAndComments(tokenizer);
  vect.z = CopyAllocCharsUntillTok(TOK_RBRACK, tokenizer, stack);

  return vect;
}

char* ParseAbsoluteRelative(Tokenizer *tokenizer, StackAllocator *stack) {
  char *result = NULL;
  Token token;
  if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);

  if (TokenEquals(&token, "ABSOLUTE")) {
    AllocTokenValue(&result, &token, stack);
  }
  else if (TokenEquals(&token, "RELATIVE")) {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
    AllocTokenValue(&result, &token, stack);
  }
  else {
    PrintLineError(tokenizer, &token, "Expected RELATIVE [compname/PREVIOUS] or ABSOLUTE.");
    exit(1);
  }
  return result;
}


ArrayListT<CompDecl> ParseTraceComps(Tokenizer *tokenizer, StackAllocator *stack) {
  ArrayListT<CompDecl> result;
  Tokenizer save = *tokenizer;
  Token token = {};

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
        if (TokenEquals(&token, "COMPONENT")) {
          ++count;
        }
        else if (TokenEquals(&token, "FINALIZE")) {
          parsing = false;
        }
        else if (TokenEquals(&token, "END")) {
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
    if (LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      break;
    }
    else {
      CompDecl comp = {};

      // split [OPTIONAL]
      if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "SPLIT")) {
        token = GetToken(tokenizer);
        comp.split = 1;
        if (LookAheadNextToken(tokenizer, TOK_INT)) {
          token = GetToken(tokenizer);
          char split[10];
          strncpy(split, token.text, token.len);
          comp.split = atoi(split);
          if (comp.split == 0) {
            PrintLineError(tokenizer, &token, "Expected a positive integer");
            exit(1);
          }
        }
      }
      
      // declaration
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "COMPONENT")) exit(0);
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
      AllocTokenValue(&comp.name, &token, stack);
      if (!RequireToken(tokenizer, &token, TOK_ASSIGN)) exit(0);
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
      AllocTokenValue(&comp.type, &token, stack);

      // params
      if (!RequireToken(tokenizer, &token, TOK_LBRACK)) exit(0);

      comp.params = ParseCompParams(tokenizer, stack);
      if (!RequireToken(tokenizer, &token, TOK_RBRACK)) exit(0);

      // location / AT
      if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "AT")) exit(0);
      RequireToken(tokenizer, &token, TOK_LBRACK);
      comp.at = ParseVector3(tokenizer, stack);
      RequireToken(tokenizer, &token, TOK_RBRACK);
      comp.at_relative = ParseAbsoluteRelative(tokenizer, stack);

      // rotation / ROTATED [OPTIONAL]
      if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "ROTATED")) {
        token = GetToken(tokenizer);
        RequireToken(tokenizer, &token, TOK_LBRACK);
        comp.rot = ParseVector3(tokenizer, stack);
        RequireToken(tokenizer, &token, TOK_RBRACK);
        comp.rot_relative = ParseAbsoluteRelative(tokenizer, stack);
      }

      lst.Add(&comp);
    }
  }

  return lst;
}

void ParseInstrument(Tokenizer *tokenizer, StackAllocator *stack) {
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

    printf("instr name:\n%s\n\n", instr.name);
    printf("instr params:\n");
    for (int i = 0; i < instr.params.len; i++) {
      InstrParam* p = instr.params.At(i);
      printf("  %s %s = %s\n", p->type, p->name, p->defaultval);
    }
    printf("\n");
  }

  // declare
  if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "DECLARE")) {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "DECLARE")) exit(1);

    DeclareDef declare;
    instr.declare = declare;
    instr.declare.text = CopyPercentBraceTextBlock(tokenizer, stack);

    ArrayListT<StructMember> lst = ParseStructMembers(instr.declare.text, stack);
    
    printf("declare members:\n");
    for (int i = 0; i < lst.len; ++i) {
      StructMember *memb = lst.At(i);
      printf("  %s %s = %s\n", memb->type, memb->name, memb->defval);
    }
    printf("\n");
  }

  // initialize
  if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "INITIALIZE")) {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "INITIALIZE")) exit(1);

    InitializeDef init;
    instr.init = init;
    instr.init.text = CopyPercentBraceTextBlock(tokenizer, stack);

    printf("init text:%s\n\n", instr.init.text);
  }

  // trace
  {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "TRACE")) exit(1);

    TraceDef trace;
    instr.trace = trace;
    Tokenizer save = *tokenizer;
    instr.trace.text = GetTextUntillEOSOrBlockClose(tokenizer, stack);
    *tokenizer = save;
    trace.comps = ParseTraceComps(tokenizer, stack);

    printf("components:\n");
    auto lst = trace.comps;
    for (int i = 0; i < lst.len; ++i) {
      CompDecl *comp = lst.At(i);
      printf("  type: %s\n", comp->type);
      printf("  name %s\n", comp->name);
      printf("  params:\n");
      auto lstp = comp->params;
      for (int i = 0; i < lstp.len; ++i) {
        CompParam *param = lstp.At(i);
        printf("    name: %s\n", param->name);
        printf("    value %s\n", param->value);
      }
      printf("  at:      (%s, %s, %s) %s\n", comp->at.x, comp->at.y, comp->at.z, comp->at_relative);
      printf("  rotated: (%s, %s, %s) %s\n", comp->rot.x, comp->rot.y, comp->rot.z, comp->rot_relative);
    }
    printf("\n");
  }

  // finalize
  if (LookAheadNextToken(tokenizer, TOK_IDENTIFIER, "FINALIZE")) {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "FINALIZE")) exit(0);

    FinalizeDef finalize;
    instr.finalize = finalize;
    instr.finalize.text = CopyPercentBraceTextBlock(tokenizer, stack);

    printf("finalize text:%s\n\n", instr.finalize.text);
  }
}


void TestParseMcStas(int argc, char **argv) {
  //char *filename = (char*) "PSI_DMC.instr";
  char *filename = (char*) "PSI.instr";
  char *text = LoadFile(filename, true);
  if (text == NULL) {
      printf("could not load file %s\n", filename);
      exit(1);
  }
  printf("parsing file %s\n\n", filename);

  StackAllocator stack(MEGABYTE);

  Tokenizer tokenizer = {};
  tokenizer.Init(text);

  ParseInstrument(&tokenizer, &stack);
}
