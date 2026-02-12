BIN=philosophers


all: clean
	mkdir build
	cd build && cmake .. && make ${BIN} && make test

clean:
	rm -rf build
