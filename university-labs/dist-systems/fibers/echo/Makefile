BIN=echo


all: clean
	mkdir -p build
	cd build && cmake .. && make ${BIN} && make test

clean:
	rm -rf build
