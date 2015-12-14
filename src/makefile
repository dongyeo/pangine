objects = pangine.o csapp.o
pangine : $(objects)
	gcc -o pangine $(objects) -lpthread
pangine.o : pangine.c csapp.h
	gcc -c pangine.c
csapp.o : csapp.c csapp.h
	gcc -c csapp.c
clean :
	rm *.o pangine
