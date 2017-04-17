libboxer.a: headless.o
	make -f Makefile-common $@

linux:
	make -f Makefile-linux

headless.o: headless.cpp
	g++ -g -c $< -o $@ -I../../include -std=c++11

clean:
	make -f Makefile-common $@
