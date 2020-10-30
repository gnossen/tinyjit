%.o: %.S
	# as $< -o $@ -f elf64
	gcc -c $< -o $@

runner.o: runner.cc
	g++ -g -c $< -o $@

%.regex: %.o runner.o
	g++ -o $@ $^

.PHONY: clean
clean:
	rm -f *.bin
