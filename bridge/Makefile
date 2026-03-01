CC = gcc

CFLAGS = -Wall -I../src -I.

TEST_DIR = tests

BRIDGE_SRC = bridge-minecraft.c
TEST_SRC = $(TEST_DIR)/test_bridge_minecraft.c

TEST_EXE := $(notdir $(TEST_SRC))
TEST_EXE := $(basename $(TEST_EXE))

all: $(TEST_EXE)

$(TEST_EXE): $(TEST_SRC) $(BRIDGE_SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TEST_EXE)