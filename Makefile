# You can reset TARGET to sdb if you want
TARGET := hw4

CC := gcc
CFLAGS := -I include/

BUILD_DIR = build

C_FILES = $(shell find "./src" -name "*.c")
O_FILES = $(patsubst ./src/%.c,./build/%.o,$(C_FILES))

all: $(TARGET)

$(TARGET): $(O_FILES)
	gcc $^ -o $@

build/%.o: src/%.c
	@mkdir -p $(@D)
	gcc -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(O_FILES) $(TARGET)