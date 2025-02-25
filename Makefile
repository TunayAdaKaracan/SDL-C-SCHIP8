build:
	clang ./src/*.c -o ./out/main -lSDL2 -Wall -Wextra

clean:
	rm ./out/main

run:
	./out/main

compile: build run
