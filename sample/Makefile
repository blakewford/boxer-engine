include ../boxer.mk

sample: main.o
	g++ $^ -O3 -o sample -L$(BOXER_DIR)/lib -lboxer -lpthread -lncurses

linux: main.o
	g++ $^ -O3 -o sample -L$(BOXER_DIR)/lib -lboxer -lpthread $(BOXER_LIBS)
.PHONY: dictionary.h

dictionary.h:
ifndef BOXER_DIR
    $(error BOXER_DIR is undefined)
endif
	$(BOXER_DIR)/tools/gen > $@

main.o: main.cpp dictionary.h
	g++ -g -c $< -o $@ -I$(BOXER_DIR)/include -std=c++11

clean:
	-@rm sample
	-@rm dictionary.h
	-@rm *.o
