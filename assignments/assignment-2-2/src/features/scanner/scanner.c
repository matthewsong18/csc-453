#include "scanner.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_OP_LEN 3

// Global lexer variables.
char *lexeme;
int lval;

// Function prototypes.
void skip_whitespace_and_comments(void);
static char *build_token_candidate(void);
static const TokenMatch *find_matching_token(char *candidate, int candidate_len,
                                             int *matched_length);
static void push_back_extra_chars(const char *candidate, int from,
                                  int candidate_len);

static int is_whitespace(int character);
static int handle_comment_start(void);
static void skip_multi_line_comment(void);

// Token registry (sorted by match priority)
static const TokenMatch *token_types[] = {
    // --- 1. Keywords (exact matches first) ---
    &M_kwINT,
    &M_kwIF,
    &M_kwELSE,
    &M_kwWHILE,
    &M_kwRETURN,

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
    NULL,
};

int currentLine = 1;

//
// get_token()
// 1. Skips leading whitespace/comments.
// 2. Builds a candidate token string from input.
// 3. Finds the longest valid match among the TokenMatch functions.
// 4. Pushes back any extra characters that were read.
// 5. Returns the token value (or UNDEF for an unknown token).
//
int get_token(void) {
  skip_whitespace_and_comments();

  char *candidate = build_token_candidate();
  if (!candidate)
    return EOF; // End-of-file reached

  int candidate_len = strlen(candidate);
  int matched_length = 0;
  const TokenMatch *matched_token =
      find_matching_token(candidate, candidate_len, &matched_length);

  if (matched_token) {
    push_back_extra_chars(candidate, matched_length, candidate_len);
    candidate[matched_length] = '\0';
    lexeme = candidate;
    lval = matched_token->value;
    return matched_token->value;
  } else {
    // No valid token was found; consume only the first character.
    push_back_extra_chars(candidate, 1, candidate_len);
    candidate[1] = '\0';
    return UNDEF;
  }
}

//
// find_matching_token()
// Try candidate prefixes from the full length down to 1 and return the
// first (i.e. longest) match found. The matched length is stored in
// *matched_length.
//
static const TokenMatch *find_matching_token(char *candidate, int candidate_len,
                                             int *matched_length) {
  const TokenMatch *found = NULL;
  for (int len = candidate_len; len >= 1; len--) {
    char saved =
        candidate[len];    // Temporarily save the character at position len.
    candidate[len] = '\0'; // Temporarily terminate the candidate here.

    for (const TokenMatch **token = token_types; *token; token++) {
      const char *result = (*token)->match(candidate);
      if (result && (result - candidate) == len) {
        *matched_length = len;
        found = *token;
        break;
      }
    }
    candidate[len] = saved; // Restore the candidate.

    if (found)
      break;
  }
  return found;
}

//
// push_back_extra_chars()
// Pushes back characters from candidate[from] up through
// candidate[candidate_len-1] into the input stream so that they are available
// for future tokenization.
//
static void push_back_extra_chars(const char *candidate, int from,
                                  int candidate_len) {
  for (int i = candidate_len - 1; i >= from; i--) {
    ungetc(candidate[i], stdin);
  }
}

//
// build_token_candidate()
// Reads input and builds a candidate string.
// - For alphanumeric characters, reads until a non-alnum is encountered.
// - For other characters, attempts to read up to MAX_OP_LEN characters
//   if they belong to a set of operator characters.
//
static char *build_token_candidate(void) {
  static char buffer[1024];
  int pos = 0;

  int ch = getchar();
  if (ch == EOF)
    return NULL;

  if (ch == '\n')
    currentLine++;

  buffer[pos++] = ch;

  if (isalnum(ch)) {
    // Read full identifier or number.
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
    // For non-alphanumerics, attempt to read a multi-character operator.
    const char *op_chars = "=!<>|&+-*/%";
    if (strchr(op_chars, ch)) {
      for (int i = 1; i < MAX_OP_LEN && pos < 1023; i++) {
        int next = getchar();
        if (next == EOF)
          break;
        if (isspace(next) || isalnum(next) || !strchr(op_chars, next)) {
          ungetc(next, stdin);
          break;
        }
        buffer[pos++] = next;
      }
    }
    // For punctuation (like '(' or ')'), we simply keep the single character.
  }
  buffer[pos] = '\0';
  return buffer;
}

//
// skip_whitespace_and_comments()
// Consumes all whitespace and comments from the input before the next token.
//
void skip_whitespace_and_comments(void) {
  int character;
  while ((character = getchar()) != EOF) {
    if (character == '\n')
      currentLine++;

    if (is_whitespace(character))
      continue;
    if (character == '/' && handle_comment_start())
      continue;
    else if (character == '/')
      return;

    ungetc(character, stdin);
    return;
  }
}

//
// is_whitespace()
// Returns nonzero if the character is a whitespace character.
//
static int is_whitespace(int character) {
  return character == ' ' || character == '\t' || character == '\n' ||
         character == '\r';
}

//
// handle_comment_start()
// Checks if the '/' character starts a comment. If the next character is '*',
// it skips the multi-line comment and returns 1; otherwise, it pushes the
// character(s) back and returns 0.
//
static int handle_comment_start(void) {
  int next_char = getchar();

  if (next_char == '*') {
    skip_multi_line_comment();
    return 1;
  }

  ungetc(next_char, stdin);
  ungetc('/', stdin);
  return 0;
}

//
// skip_multi_line_comment()
// Consumes characters until the closing '*/' is found.
//
static void skip_multi_line_comment(void) {
  int previous = 0;
  int current = 0;

  while ((current = getchar()) != EOF) {
    if (previous == '*' && current == '/')
      return;
    previous = current;
  }
}
