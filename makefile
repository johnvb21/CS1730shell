cl: shell.c
	gcc -o shell shell.c -Wall -pedantic-errors
run: cl
	./shell
clean:
	rm -f shell
