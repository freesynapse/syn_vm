GCC=gcc
CFLAGS=-Wall -Wextra -std=c++23
LDFLAGS=

.PHONY=all
all: synvm sasm

synvm: ./src/syn_vm.cpp ./src/vm.h ./src/string_view.h
	$(GCC) $(CFLAGS) ./src/syn_vm.cpp -o synvm $(LDFLAGS)

sasm: ./src/sasm.cpp ./src/vm.h ./src/string_view.h
	$(GCC) $(CFLAGS) ./src/sasm.cpp -o sasm $(LDFLAGS)

.PHONY=examples
#examples:


clean:
	rm synvm sasm


