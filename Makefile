CC=clang
CFLAGS=-g -Wall -Wextra

# compiler-specific flags
ifeq ($(CC),gcc)
        CFLAGS+=-fanalyzer
else ifeq ($(CC),clang)
        #CFLAGS+=--analyze
else
endif

# debug-specific flag
ifndef DEBUG
        # undefined build
        CFLAGS+=-O2
else ifeq ($(DEBUG),0)
        # release
        CFLAGS+=-O3 -DDEBUG=0
else ifeq ($(DEBUG),1)
        # debug
        CFLAGS+=-O0 -DDEBUG=1
else ifeq ($(DEBUG),2)
        # sanitized
        CFLAGS+=-O0 -DDEBUG=1
        ifeq ($(CC),gcc)
                # gcc sanitizer
                CFLAGS+=-fsanitize=address
        else ifeq ($(CC),clang)
                # clang sanitizer
                CFLAGS+=-fsanitize=address
        else
        endif
else
        # unknown debugness!
        $(warning unknown value of $$DEBUG - {$(DEBUG)})
endif

build:
	$(CC) $(CFLAGS) ./src/*.c -o ./out/main -lSDL2

clean:
	rm ./out/main

run:
	./out/main $(PATH)

compile: build run
