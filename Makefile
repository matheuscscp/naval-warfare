
all: run

configure:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE=Debug ../; cd ..

build: configure
	make -C build

install: build
	make -C build install

run: install
	gear2d partida.yaml

gdb: install
	gdb gear2d

clean:
	rm -rf build components

.PHONY: all build install run gdb

