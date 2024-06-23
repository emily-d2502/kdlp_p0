COMPILER := clang
COMPILER_FLAGS := -std=c99 -Wextra -Wpedantic
SRCS := shell.c utilities.c
OBJS=$(subst .c,.o,$(SRCS))
HDRS := main.h

eshell: $(OBJS)
	$(COMPILER) $(COMPILER_FLAGS) $^ -o $@

$(OBJS): %.o: %.c
	$(COMPILER) $(COMPILER_FLAGS) -c $^

zip: $(SRCS) $(HDRS)
	zip shell.zip $^ Makefile

clean:
	rm -rf eshell $(OBJS)
	rm -rf shell.zip
