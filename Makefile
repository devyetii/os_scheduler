build:
	gcc process_generator.c -o process_generator.out -lm
	gcc clk.c -o clk.out -lm
	gcc scheduler.c -o scheduler.out -lm
	gcc process.c -o process.out -lm
	gcc test_generator.c -o test_generator.out -lm

clean:
	ipcrm -a
	rm -f *.out

all: clean build

run:
	./process_generator.out
memtest:
	rm -f try
	gcc -o try try.c
	valgrind --tool=memcheck ./try