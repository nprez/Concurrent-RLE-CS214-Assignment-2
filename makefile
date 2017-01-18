ALL:thread process
thread:
	gcc -g -Wall -pthread -lm compressT_LOLS.c -o compressT_LOLS.out
process:
	gcc -g -Wall -lm compressR_LOLS.c -o compressR_LOLS.out
	gcc -g -Wall compressR_worker_LOLS.c -o compressR_worker_LOLS.out
clean:
	rm -rf *.out
	rm -rf *_txt_LOLS*