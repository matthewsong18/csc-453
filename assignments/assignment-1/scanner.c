#include "scanner.h"
#include <ctype.h>
#include <stdio.h>

char *lexeme;
int lval;

void skip_whitespace_and_comments();
static int is_whitespace(int character);
static int handle_comment_start();
static void skip_multi_line_comment();

// Token registry (sorted by match priority)
static const TokenMatch *token_types[] = {
    // --- 1. Keywords (exact matches first) ---
    &M_kwINT, &M_kwIF, &M_kwELSE, &M_kwWHILE, &M_kwRETURN,

    // --- 2. Multi-character operators ---
    &M_opEQ,  // ==
    &M_opNE,  // !=
    &M_opGE,  // >=
    &M_opLE,  // <=
    &M_opAND, // &&
    &M_opOR,  // ||

    // --- 3. Single-character operators ---
    &M_opASSG, // =
    &M_opADD,  // +
    &M_opSUB,  // -
    &M_opMUL,  // *
    &M_opDIV,  // /
    &M_opGT,   // >
    &M_opLT,   // <
    &M_opNOT,  // !

    // --- 4. Brackets/punctuation ---
    &M_LPAREN, // (
    &M_RPAREN, // )
    &M_LBRACE, // {
    &M_RBRACE, // }
    &M_COMMA,  // ,
    &M_SEMI,   // ;

    // --- 5. Complex patterns ---
    &M_INTCON, // Integer constants
    &M_ID,     // Identifiers

    // --- Terminator ---
    NULL};

int get_token() {
  static char buffer[1024];  // Buffer to store current token text
  static int buffer_pos = 0; // Current position in buffer

  // Skip any leading whitespace or comments
  skip_whitespace_and_comments();

  int character = getchar();
  if (character == EOF)
    return -1;

  // Start filling buffer with first character
  buffer_pos = 0;
  buffer[buffer_pos++] = character;

  // If it's a letter or number, read until we reach a non-identifier character
  if (isalnum(character)) {
    while (buffer_pos < 1023) {
      int next = getchar();
      if (!isalnum(next)) {
        ungetc(next, stdin);
        break;
      }
      buffer[buffer_pos++] = next;
    }
  }
  buffer[buffer_pos] = '\0';

  // Try to match against each token pattern
  for (const TokenMatch **token = token_types; *token; token++) {
    const char *match_result = (*token)->match(buffer);
    if (match_result) {
      // Store token information in global variables
      lexeme = buffer;
      lval = (*token)->value;
      return (*token)->value;
    }
  }

  // Undefined
  buffer_pos = 0;
  buffer[buffer_pos++] = character; // Store only the first unknown character
  buffer[buffer_pos] = '\0';        // Null-terminate the buffer
  return UNDEF;
}

void skip_whitespace_and_comments() {
  int character;

  while ((character = getchar()) != EOF) {
    if (is_whitespace(character)) {
      continue;
    }

    if (character == '/' && handle_comment_start()) {
      continue;
    } else if (character == '/') {
      return;
    }

    ungetc(character, stdin);
    return;
  }
}

static int is_whitespace(int character) {
  return character == ' ' || character == '\t' || character == '\n' ||
         character == '\r';
}

static int handle_comment_start() {
  int next_char = getchar();

  if (next_char == '*') {
    skip_multi_line_comment();
    return 1;
  }

  ungetc(next_char, stdin);
  ungetc('/', stdin);
  return 0;
}

static void skip_multi_line_comment() {
  int current = 0;
  int previous = 0;

  while ((current = getchar()) != EOF) {
    if (previous == '*' && current == '/') {
      return;
    }
    previous = current;
  }
}
