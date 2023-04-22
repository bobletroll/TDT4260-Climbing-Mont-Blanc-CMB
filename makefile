run:
	@gcc -g3 -fno-omit-frame-pointer -Og -fopenmp -std=c99 main.c -o out.bin -lpthread
	@time ./out.bin 1

debug:
	@gdb -ex=r --args out.bin 1

zip:
	@zip submission.zip main.c