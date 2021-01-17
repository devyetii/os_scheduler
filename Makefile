build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out -lm
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	ipcrm -a
	rm -f *.out

all: clean build

run:
	ipcrm -a
	pkill -f clk.out
	./process_generator.out

memtest:
	rm -f try
	gcc -o try try.c
	valgrind --tool=memcheck ./try