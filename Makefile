all: ip6Parse-test

clean:
	rm -f ip6Parse-test *.exe

ip6Parse-test: ip6Parse.c
	${CC} -Wall -DHAS_MAIN -o ip6Parse-test ip6Parse.c
