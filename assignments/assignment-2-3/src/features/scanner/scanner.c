#include "scanner.h"
#include <ctype.h>
#include <stdbool.h>
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
static void skip_multi_line_comment(void);

int currentLine = 1;

static const char *input_buffer = NULL;
static int buffer_pos = 0;
static bool reading_from_string = false;
static int string_pushed_back_char = -1; // Buffer for one ungetc from string

// Replacement for getchar()
static int scanner_getchar() {
  int ch;
  if (reading_from_string) {
    if (string_pushed_back_char != -1) {
      ch = string_pushed_back_char;
      string_pushed_back_char = -1;
      return ch;
    }
    if (input_buffer && input_buffer[buffer_pos] != '\0') {
      ch = input_buffer[buffer_pos++];
    } else {
      ch = EOF;
    }
  } else {
    ch = getchar();
  }
  if (ch == '\n') {
    currentLine++;
  }
  return ch;
}

// Replacement for ungetc()
static void scanner_ungetc(int c) {
  if (c == EOF)
    return;

  // Decrement line count before pushback if it's a newline
  // This assumes the newline was the *last* char read. Be wary of edge cases.
  if (c == '\n' && currentLine > 1) { // Avoid going below line 1
    currentLine--;
  }

  if (reading_from_string) {
    // Try to "push back" by moving buffer pointer
    if (buffer_pos > 0 && string_pushed_back_char == -1) {
      buffer_pos--;
    }
    // Use the single-char pushback buffer if pointer can't move back
    else if (string_pushed_back_char == -1) {
      string_pushed_back_char = c; // Store the pushed-back char
    } else {
      // Error: Cannot push back more than one character reliably from string
      fprintf(stderr, "Scanner Error: Cannot ungetc character from string "
                      "input (buffer full or at start).\n");
    }
  } else {
    ungetc(c, stdin);
  }
}

void scanner_init_with_string(const char *input_string) {
  input_buffer = input_string;
  buffer_pos = 0;
  reading_from_string = true;
  string_pushed_back_char = -1;
  currentLine = 1;
}

void scanner_init_with_stdin(void) {
  input_buffer = NULL;
  reading_from_string = false;
  string_pushed_back_char = -1;
  currentLine = 1;
}

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
    scanner_ungetc(candidate[i]);
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

  int ch = scanner_getchar();
  if (ch == EOF)
    return NULL;

  if (ch == '\n')
    currentLine++;

  buffer[pos++] = ch;

  if (isalnum(ch)) {
    // Read full identifier or number.
    while (pos < 1023) {
      int next = scanner_getchar();
      if (next == EOF || !isalnum(next)) {
        if (next != EOF)
          scanner_ungetc(next);
        break;
      }
      buffer[pos++] = next;
    }
  } else {
    // For non-alphanumerics, attempt to read a multi-character operator.
    const char *op_chars = "=!<>|&+-*/%";
    if (strchr(op_chars, ch)) {
      for (int i = 1; i < MAX_OP_LEN && pos < 1023; i++) {
        int next = scanner_getchar();
        if (next == EOF || !strchr(op_chars, next)) {
          if (next != EOF)
            scanner_ungetc(next);
          break;
        }
        buffer[pos++] = next;
      }
    }
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
  while ((character = scanner_getchar()) != EOF) {
    if (is_whitespace(character))
      continue;

    if (character == '/') {
      int next_char = scanner_getchar();
      if (next_char == '*') {
        skip_multi_line_comment();
        continue;
      } else {
        scanner_ungetc(next_char);
        scanner_ungetc(character);
        return;
      }
    }
    scanner_ungetc(character);
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
// skip_multi_line_comment()
// Consumes characters until the closing '*/' is found.
//
static void skip_multi_line_comment(void) {
  int previous = 0;
  int current = 0;
  while ((current = scanner_getchar()) != EOF) {
    if (previous == '*' && current == '/')
      return;
    previous = current;
  }
}
