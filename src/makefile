objects = threadPool.o pangine.c
pangine : $(objects)
	gcc -o pangine $(objects) -lpthread -g
threadPool.o : threadPool.c threadPool.h
	gcc -c threadPool.c
.PHONY : clean install
clean :
	rm *.o pangine
install :
	-mkdir $(DESTDIR)
	-mkdir $(DESTDIR)/bin
	cp ./pangine $(DESTDIR)/bin
