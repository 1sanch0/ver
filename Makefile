DEBUG = 1

CC = g++
EMCC = em++

SRC_DIR = ./src
INCLUDE_DIR = ./include

CFLAGS = -std=c++17 -I$(SRC_DIR) -isystem $(INCLUDE_DIR)
LFLAGS = -lm

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g3
else
	CFLAGS += -O3 -DNDEBUG
endif

# Warnings
CFLAGS += -Wall -Wextra -Wpedantic -Wcast-qual -Wshadow -Wpointer-arith

# Courtesy of: https://stackoverflow.com/a/12959764
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

CC_FILES = $(call rwildcard,$(SRC_DIR),*.cc) # Spaces after commas makes it return every file and directory LULE
OBJECTS = $(patsubst %.cc, %.o, $(CC_FILES))

ver: CFLAGS += -fopenmp -march=native -mtune=native

ver: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.cc %.hh
	$(CC) $(CFLAGS) -c $< -o $@

run: ver
	./ver

# Profiling
callgrind: ver
	valgrind --tool=callgrind ./$<

.PHONY: clean
clean:
	$(RM) ver *.out.* $(OBJECTS)
