build:
	gcc -Wall -Wextra -o singlethreaded file_processor_singlethreaded.c

	gcc -Wall -Wextra -o multithreaded file_processor_multithreaded.c -lpthread

runs:
	./singlethreaded data_tiny.txt

runm:
	./multithreaded data_tiny.txt 4

clean:
	rm -f singlethreaded multithreaded 

