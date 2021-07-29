#include "memory.h"
#include "token.h"


//
// parsing C structs


struct StructMember {
  char *type = NULL;
  char *name = NULL;
  char *defval = NULL;
};


ArrayListT<StructMember> ParseStructMembers(Tokenizer *tokenizer, StackAllocator *stack) {
  // count the number of members by counting semi-colons
  bool parsing = true;
  u32 count = CountTokenSeparatedStuff(tokenizer->at, TOK_SEMICOLON);

  ArrayListT<StructMember> lst;
  lst.Init(stack->Alloc(sizeof(StructMember) * count));
  Token token;

  while (true) {
    StructMember member;

    if (!LookAheadNextToken(tokenizer, TOK_IDENTIFIER)) {
      break;
    }

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
    AllocTokenValue(&member.type, &token, stack);

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(1);
    AllocTokenValue(&member.name, &token, stack);

    if (LookAheadNextToken(tokenizer, TOK_ASSIGN)) {
      token = GetToken(tokenizer);
      EatWhiteSpacesAndComments(tokenizer);

      u32 len = LookAheadLenUntilToken(tokenizer, TOK_SEMICOLON, true);
      u32 len_eol = LookAheadLenEoL(tokenizer->at);

      if (RequireToken(tokenizer, NULL, TOK_SEMICOLON, NULL, false)) {
        PrintLineError(tokenizer, NULL, "Expected default value");
        exit(1);
      }
      else if (len == 0 || len_eol < len) {
        PrintLineError(tokenizer, NULL, "Expected ;");
        exit(1);
      }
      member.defval = (char*) stack->Alloc(len + 1);
      strncpy(member.defval, tokenizer->at, len);
      member.defval[len] = '\0';
      tokenizer->at += len;
    }

    if (!RequireToken(tokenizer, NULL, TOK_SEMICOLON)) exit(1);
    lst.Add(&member);

    // positive exit condition
    if (LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
      break;
    }
    else if (LookAheadNextToken(tokenizer, TOK_RPERBRACE)) {
      break;
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
  char *extend = NULL;
  u32 split = 0;
  ArrayListT<CompParam> params;
  Vector3Strings at;
  Vector3Strings rot;
  char *at_relative = NULL; // value can be ABSOLUTE or a comp name
  char *rot_relative = NULL; // value must be a comp name
};

struct TraceDef {
  char *text = NULL;
  ArrayListT<CompDecl> comps;
};

struct InstrDef {
  char *name = NULL;
  ArrayListT<InstrParam> params;
  DeclareDef declare;
  InitializeDef init;
  TraceDef trace;
  FinalizeDef finalize;
};

ArrayListT<InstrParam> ParseInstrParams(Tokenizer *tokenizer, StackAllocator *stack) {
  u32 count = CountTokenSeparatedStuff(tokenizer->at, TOK_COMMA, TOK_RBRACK);

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

char* CopyBracketedTextBlock(Tokenizer *tokenizer, TokenType type_start, TokenType type_end, bool restore_tokenizer, StackAllocator *stack) {
  Tokenizer save = *tokenizer;

  if (type_start != TOK_UNKNOWN) {
    if (!RequireToken(tokenizer, NULL, type_start)) exit(1);
  }

  char *text_start = tokenizer->at;
  LTrim(&text_start);

  Token token;
  char *text = NULL;

  u32 dist = LookAheadLenUntilToken(tokenizer, type_end);
  if (dist == 0 && LookAheadNextToken(tokenizer, TOK_ENDOFSTREAM)) {
    PrintLineError(tokenizer, &token, "End of file reached");
    exit(1);
  }
  u32 last_token_len = 0;
  while (token.type != type_end) {
    last_token_len = token.len;
    token = GetToken(tokenizer);
  }

  u32 len_untrimmed = (tokenizer->at - 1) - text_start - token.len;
  u32 len = RTrimText(text_start, len_untrimmed);

  text = (char*) stack->Alloc(len + 1);
  strncpy(text, text_start, len);
  text[len] = '\0';

  if (restore_tokenizer == true) {
    *tokenizer = save;
  }

  return text;
}

ArrayListT<CompParam> ParseCompParams(Tokenizer *tokenizer, StackAllocator *stack) {
  u32 count = CountTokenSeparatedStuff(tokenizer->at, TOK_COMMA, TOK_RBRACK);

  ArrayListT<CompParam> lst;
  lst.Init(stack->Alloc(sizeof(CompParam) * count));

  CompParam par;
  Token token;

  for (int i = 0; i < count; i++) {
    CompParam par;

    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER)) exit(0);
    AllocTokenValue(&par.name, &token, stack);

    if (!RequireToken(tokenizer, &token, TOK_ASSIGN)) exit(0);

    EatWhiteSpacesAndComments(tokenizer);
    u32 vallen = MinU(LookAheadLenUntilToken(tokenizer, TOK_COMMA), LookAheadLenUntilToken(tokenizer, TOK_RBRACK));
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

char* CopyAllocCharsUntillTok(TokenType token_type, Tokenizer *tokenizer, StackAllocator *stack) {
  char* result;
  u32 len = LookAheadLenUntilToken(tokenizer, token_type);
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
      if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "SPLIT")) {
        comp.split = 1;
        if (RequireToken(tokenizer, &token, TOK_INT)) {
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
      if (RequireToken(tokenizer, NULL, TOK_IDENTIFIER, "ROTATED")) {
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

  // declare
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "DECLARE", false)) {
    instr.declare.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, true, stack);

    if (!RequireToken(tokenizer, &token, TOK_LPERBRACE)) exit(1);
    instr.declare.decls = ParseStructMembers(tokenizer, stack);
    if (!RequireToken(tokenizer, &token, TOK_RPERBRACE)) exit(1);
  }

  // initialize
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "INITIALIZE", false)) {
    instr.init.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
  }

  // trace
  {
    if (!RequireToken(tokenizer, &token, TOK_IDENTIFIER, "TRACE")) exit(1);
    instr.trace.text = CopyBracketedTextBlock(tokenizer, TOK_UNKNOWN, TOK_MCSTAS_END, true, stack);

    instr.trace.comps = ParseTraceComps(tokenizer, stack);
  }

  // finalize
  if (RequireToken(tokenizer, &token, TOK_IDENTIFIER, "FINALLY", false)) {
    instr.finalize.text = CopyBracketedTextBlock(tokenizer, TOK_LPERBRACE, TOK_RPERBRACE, false, stack);
  }

  return instr;
}

void PrintInstrumentParse(InstrDef instr) {
  printf("instr name:\n%s\n\n", instr.name);
  printf("instr params:\n");
  for (int i = 0; i < instr.params.len; i++) {
    InstrParam* p = instr.params.At(i);
    printf("  %s %s = %s\n", p->type, p->name, p->defaultval);
  }
  printf("\n");

  printf("declare members:\n");
  for (int i = 0; i < instr.declare.decls.len; ++i) {
    StructMember *memb = instr.declare.decls.At(i);
    printf("  %s %s = %s\n", memb->type, memb->name, memb->defval);
  }
  printf("\n");

  printf("init text:\n%s\n\n", instr.init.text);

  printf("components:\n");
  for (int i = 0; i < instr.trace.comps.len; ++i) {
    CompDecl *comp = instr.trace.comps.At(i);
    printf("\n  type: %s\n", comp->type);
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

  printf("finalize text:\n%s\n\n", instr.finalize.text);
}

void TestParseMcStas(int argc, char **argv) {
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

  InstrDef instr = ParseInstrument(&tokenizer, &stack);

  PrintInstrumentParse(instr);
}
