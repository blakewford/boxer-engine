libboxer.a: boxer.o stage.o linux.o
	ar -cvq $@ $^
	cp $@ ../$@

boxer.o: boxer.cpp
	g++ -g -c $< -o $@ -I../../include -std=c++11

stage.o: stage.cpp
	g++ -g -c $< -o $@ -I../../include -std=c++11

linux.o: linux.cpp
	g++ -g -c $< `pkg-config --cflags --libs gtk+-3.0` -o $@ -I../../include -std=c++11

clean:
	-@rm libboxer.a
	-@rm *.o
