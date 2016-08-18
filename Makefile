PEBBLE_HEADERS=tests/include/pebble/

CC=gcc
ifeq ($(TRAVIS), true)
CC_TEST=$(CC)
CFLAGS=-std=c99
AR=ar
else
CC_TEST=arm-none-eabi-gcc
CFLAGS=-std=c11
AR=arm-none-eabi-ar
endif
CINCLUDES=-I $(PEBBLE_HEADERS) -I tests/ -I tests/generated/ -I tests/include/ -I tests/include/@smallstoneapps/linked-list

TEST_FILES=tests/enamel.c
SRC_FILES=tests/generated/enamel.c tests/linked-list.c
TEST_EXTRAS=tests/pebble_stub.c tests/pebble-events_stub.c

all: test

test:
	@$(CC) $(CFLAGS) $(CINCLUDES) $(TEST_FILES) $(SRC_FILES) $(TEST_EXTRAS) -o tests/run
	@tests/run
	@rm tests/run