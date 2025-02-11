#include "scanner.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_OP_LEN 3

char *lexeme;
int lval;

void skip_whitespace_and_comments();
static char *build_token_candidate();
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
  // Skip any leading whitespace or comments
  skip_whitespace_and_comments();

  char *candidate = build_token_candidate();
  if (!candidate) // End-of-file reached
    return -1;

  int candidate_len = strlen(candidate);
  int matched_length = 0;
  const TokenMatch *matched_token = NULL;

  // Try candidate prefixes from the full length down to 1.
  // (We want the longest accepted prefix.)
  for (int len = candidate_len; len >= 1; len--) {
    char saved = candidate[len]; // Save the character at the cut point.
    candidate[len] = '\0';       // Temporarily terminate the candidate.

    // Try each token in the registry.
    for (const TokenMatch **token = token_types; *token; token++) {
      const char *match_result = (*token)->match(candidate);
      // We consider it a match if the token's match function returns a pointer
      // that indicates it consumed exactly this prefix (i.e. candidate + len).
      if (match_result && (match_result - candidate) == len) {
        matched_length = len;
        matched_token = *token;
        break;
      }
    }
    candidate[len] = saved; // Restore the original character.

    if (matched_token) {
      // We found a valid token with this prefix; stop checking shorter
      // prefixes.
      break;
    }
  }

  if (matched_token) {
    // If our candidate read extra characters beyond what the token consumed,
    // push them back in reverse order so that later calls to get_token() will
    // see them.
    for (int i = candidate_len - 1; i >= matched_length; i--) {
      ungetc(candidate[i], stdin);
    }
    // Terminate the candidate at the matched length.
    candidate[matched_length] = '\0';
    lexeme = candidate;
    lval = matched_token->value;
    return matched_token->value;
  } else {
    // No token matched. Consume only the first character.
    // Push back any extra characters.
    for (int i = candidate_len - 1; i >= 1; i--) {
      ungetc(candidate[i], stdin);
    }
    // Set the candidate to just the first character.
    candidate[1] = '\0';
    return UNDEF;
  }
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

static char *build_token_candidate(void) {
  // Use a static buffer for simplicity.
  static char buffer[1024];
  int pos = 0;

  int ch = getchar();
  if (ch == EOF)
    return NULL; // End of input

  buffer[pos++] = ch;

  // If the character is alphanumeric, read until a non-alphanumeric is found.
  if (isalnum(ch)) {
    while (pos < 1023) {
      int next = getchar();
      if (next == EOF)
        break;
      if (!isalnum(next)) {
        ungetc(next, stdin);
        break;
      }
      buffer[pos++] = next;
    }
  } else {
    // For non-alphanumeric characters, we want to allow the possibility
    // of a multi-character token.
    const char *op_chars = "=!<>|&+-*/%";

    // If the first character is in our operator set, try to read up to
    // MAX_OP_LEN characters.
    if (strchr(op_chars, ch)) {
      for (int i = 1; i < MAX_OP_LEN && pos < 1023; i++) {
        int next = getchar();
        if (next == EOF)
          break;
        // Stop if we hit whitespace, an alphanumeric character, or a character
        // that is not in our operator set.
        if (isspace(next) || isalnum(next) || !strchr(op_chars, next)) {
          ungetc(next, stdin);
          break;
        }
        buffer[pos++] = next;
      }
    }
    // Otherwise, for punctuation like '(' or ')', we simply return that single
    // character.
  }
  buffer[pos] = '\0';
  return buffer;
}
