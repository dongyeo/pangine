objects = threadPool.o pangine.c
pangine : $(objects)
	gcc -o pangine $(objects) -lpthread -g
threadPool.o : threadPool.c threadPool.h
	gcc -c threadPool.c
clean :
	rm *.o pangine
