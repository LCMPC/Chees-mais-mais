CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2
TARGET := build/chess-mais-mais
SOURCES := $(shell find src -name '*.c')
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

ifeq ($(OS),Windows_NT)
  TARGET := build/chess-mais-mais.exe
  RAYLIB_CFLAGS ?=
  RAYLIB_LIBS ?= -lraylib -lopengl32 -lgdi32 -lwinmm
else
  RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib)
  RAYLIB_LIBS ?= $(shell pkg-config --libs raylib) -lm
endif

.PHONY: all run clean test
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(RAYLIB_LIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test:
	@mkdir -p build
	$(CC) $(CFLAGS) -Werror tests/testar_melhorias.c src/campo/*.c src/pecas/*.c src/pecas/unidades/*.c -o build/testar_melhorias
	./build/testar_melhorias

clean:
	rm -rf build

-include $(DEPS)
