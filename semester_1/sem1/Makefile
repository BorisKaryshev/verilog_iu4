MAKEFLAGS += --silent

all:
	cmake -S . -B build && cmake --build build -j 4
	./build/exe

.PHONY: all
