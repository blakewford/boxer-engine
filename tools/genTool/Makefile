gen: gen.cpp
	g++ -gdwarf-4 -O3 $^ -o $@ -I../../include -std=c++11
	cp $@ ../$@

clean:
	-@rm gen
