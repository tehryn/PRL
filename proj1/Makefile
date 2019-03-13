bks: bks.o
	mpic++ -std=c++11 -g bks.o -o bks

bks.o: bks.cpp bks.h
	mpic++ -std=c++11 -c bks.cpp -o bks.o

.PHONY: clean zip
clean:
	rm -f bks bks.o

zip:
	rm -f xmatej52.zip
	zip xmatej52.zip bks.cpp bks.h xmatej52.pdf test.sh