all:
	gcc 						\
		-I/usr/include/liberasurecode 		\
		-I/usr/local/include/liberasurecode	\
		-Iinclude				\
		-lerasurecode -ldl -lpthread -lz -lgcov \
		-o bin/main				\
		src/main.c

clean:
	rm -f bin/main

rotate:
	util/rotate_file.sh

run:
	util/rotate_file.sh
	util/run.sh $(be)
