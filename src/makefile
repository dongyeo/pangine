objects = threadPool.o pangine.c mySignal.o cgiRequestParams.o
pangine : $(objects)
	gcc -o pangine $(objects) -lpthread -g
threadPool.o : threadPool.c threadPool.h
	gcc -c threadPool.c
mySignal.o : mySignal.c mySignal.h
	gcc -c mySignal.c
cgiRequestParams.o : cgi.h cgiRequestParams.h cgiRequestParams.c
	gcc -c cgiRequestParams.c
.PHONY : clean install
clean :
	rm *.o pangine
install :
	-mkdir $(DESTDIR)
	-mkdir $(DESTDIR)/bin
	cp ./pangine $(DESTDIR)/bin
