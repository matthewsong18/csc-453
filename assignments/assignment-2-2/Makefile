ifeq ($(wildcard src/features),)
# --- Flat structure (submission) ---
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
INCLUDES :=
PATTERN_RULE = %.o: %.c
	CFLAGS = -Wall $(INCLUDES)
else
# --- Organized structure (development) ---
SRCDIR := src
FEATURES_DIR := $(SRCDIR)/features
PARSER_DIR := $(FEATURES_DIR)/parser
SCANNER_DIR := $(FEATURES_DIR)/scanner

SOURCES := $(wildcard $(PARSER_DIR)/*.c) \
           $(wildcard $(SCANNER_DIR)/*.c)
# Remove the 'src/' prefix for objects
OBJECTS := $(SOURCES:src/%.c=obj/%.o)
INCLUDES := -I$(PARSER_DIR) -I$(SCANNER_DIR)
PATTERN_RULE = obj/%.o: src/%.c
	CFLAGS = -Wall $(INCLUDES)
endif

CC = gcc

all: compile

compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o compile $(OBJECTS)

# Organized pattern rule:
ifeq ($(wildcard src/features),)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
else
obj/%.o: src/%.c | obj
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	
obj:
	mkdir -p obj
endif

clean:
ifeq ($(wildcard src/features),)
	rm -f $(OBJECTS) compile
else
	rm -rf obj compile
endif

submit:
	mkdir -p submission
	cp $(SOURCES) submission/
	cp $(PARSER_DIR)/*.h $(SCANNER_DIR)/*.h $(COMMON_DIR)/*.h submission/ 2>/dev/null || true
	cp Makefile submission/
