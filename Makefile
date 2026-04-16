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
examples: sasm fib iadd_mul fadd_mul pi e

iadd_mul: ./examples/iadd_mul.sasm
	./sasm -i ./examples/iadd_mul.sasm -o ./examples/iadd_mul.bin
	./synvm ./examples/iadd_mul.bin
	@echo

fadd_mul: ./examples/fadd_mul.sasm
	./sasm -i ./examples/fadd_mul.sasm -o ./examples/fadd_mul.bin
	./synvm ./examples/fadd_mul.bin
	@echo

fib: ./examples/fib.sasm
	./sasm -i ./examples/fib.sasm -o ./examples/fib.bin
	./synvm ./examples/fib.bin -l 42
	@echo

pi: ./examples/pi.sasm
	./sasm -i ./examples/pi.sasm -o ./examples/pi.bin
	./synvm ./examples/pi.bin

e: ./examples/e.sasm
	./sasm -i ./examples/e.sasm -o ./examples/e.bin
	./synvm ./examples/e.bin


clean:
	rm synvm sasm ./examples/*.bin



