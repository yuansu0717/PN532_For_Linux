

all :mydesign

mydesign :main.c ser.c ser.h funct.h funct.c
	gcc -g -Wall -O2 main.c ser.c funct.c -o mydesign
clean :
	rm -f mydesign
	rm -f *.c~
	rm -f *~
	rm -f *bak
