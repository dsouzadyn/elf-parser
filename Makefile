all: parser executable

parser: parser.c
	gcc parser.c -o parser

executable: main.c
	gcc -m32 -static main.c -o executable

clean:
	rm -vf parser executable *.o
