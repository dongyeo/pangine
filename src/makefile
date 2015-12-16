objects = pangine.o csapp.o threadPool.o
pangine : $(objects)
	gcc -o pangine $(objects) -lpthread
pangine.o : pangine.c csapp.h
	gcc -c pangine.c
csapp.o : csapp.c csapp.h
	gcc -c csapp.c
threadPool.o : threadPool.c threadPool.h
	gcc -c threadPool.c
clean :
	rm *.o pangine
