GCC=gcc
CFLAGS=-Wall -Wextra -std=c++23
LDFLAGS=

COMMON=src/common/string_view.h src/common/args.h src/common/log.h src/common/fileio.h

##
.PHONY=all
all: synvm sasm

synvm: ./src/syn_vm.cpp ./src/syn_vm.h $(COMMON)
	$(GCC) $(CFLAGS) ./src/syn_vm.cpp -o synvm $(LDFLAGS)

sasm: ./src/sasm.cpp ./src/syn_vm.h $(COMMON)
	$(GCC) $(CFLAGS) ./src/sasm.cpp -o sasm $(LDFLAGS)

##
.PHONY=examples
examples: sasm iadd fib

iadd: ./examples/test_iadd.sasm
	./sasm -i ./examples/test_iadd.sasm -o ./examples/test_iadd.bin
	./synvm ./examples/test_iadd.bin
	@echo

fib: ./examples/test_fib.sasm
	./sasm -i ./examples/test_fib.sasm -o ./examples/test_fib.bin
	./synvm ./examples/test_fib.bin -l 100
	@echo

clean:
	rm synvm sasm


