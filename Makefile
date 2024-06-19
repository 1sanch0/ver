DEBUG = 0
PLATFORM = PLATFORM_DESKTOP

CC = g++

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
viewer: CFLAGS += -DVIEWER -isystem include/raylib/src -fopenmp -march=native -mtune=native
viewer: LFLAGS += -lpthread -ldl -lX11

ver.js: CC = emcc
ver.js: PLATFORM = PLATFORM_WEB
ver.js: CFLAGS += -DVIEWER -isystem include/raylib/src
ver.js: LFLAGS += -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=64MB --preload-file assets/bunny.ply

ver: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.cc %.hh
	$(CC) $(CFLAGS) -c $< -o $@

viewer: $(OBJECTS) libraylib.a
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

ver.js: $(OBJECTS) libraylib.a
	$(CC) $(CFLAGS) $^ -o ./docs/$@ $(LFLAGS)

libraylib.a:
	cd include/ && git clone https://github.com/raysan5/raylib.git --depth=1 --branch=5.0
	cd include/raylib/src/ && make PLATFORM=$(PLATFORM)
	cp include/raylib/src/libraylib.a .

run: ver
	./ver

# Profiling
callgrind: ver
	valgrind --tool=callgrind ./$<

.PHONY: clean
clean:
	$(RM) ver viewer *.out.* $(OBJECTS) *.a
