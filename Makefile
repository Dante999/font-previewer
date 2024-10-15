all:
	mkdir -p build
	${CC} -Wall -Wextra -Wpedantic -o build/font_previewer src/main.c

clean:
	rm -r build
