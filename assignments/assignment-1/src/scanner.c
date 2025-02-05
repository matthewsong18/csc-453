#include "../include/scanner.h"
#include <stdio.h>

char *lexeme;
int lval;

void skip_whitespace_and_comments();
static int is_whitespace(int character);
static int handle_comment_start();
static void skip_single_line_comment();
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

  // Read first character of potential token
  buffer[0] = getchar();
  if (buffer[0] == EOF) {
    return 0; // End of input
  }

  // Initialize buffer with first character
  buffer_pos = 1;
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

  // No match found, backtrack and return error
  ungetc(buffer[0], stdin);
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

  if (next_char == '/') {
    skip_single_line_comment();
    return 1;
  }

  if (next_char == '*') {
    skip_multi_line_comment();
    return 1;
  }

  ungetc(next_char, stdin);
  ungetc('/', stdin);
  return 0;
}

static void skip_single_line_comment() {
  int character;
  while ((character = getchar()) != EOF && character != '\n') {
  };
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
