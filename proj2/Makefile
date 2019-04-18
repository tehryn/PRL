.PHONY: clean zip
clean:
	rm -f bks bks.o

zip:
	rm -f xmatej52.zip
	cp doc/xmatej52.pdf .
	zip xmatej52.zip vuv.cpp vuv.h xmatej52.pdf test.sh
	rm xmatej52.pdf