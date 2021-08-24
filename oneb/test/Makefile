.SILENT:


default:
	gcc -lz -g -o lab1b-server -Wextra -Wall lab1b-server.c
	gcc -lz -g -o lab1b-client -Wextra -Wall lab1b-client.c


dist: default
	tar -czvf lab1b-505103854.tar.gz lab1b-client.c lab1b-server.c README Makefile 

clean:
	rm -rf lab1b-client lab1b-server lab1b-505103854.tar.gz

server:
	gcc -lz -g -o lab1b-server -Wextra -Wall lab1b-server.c

client:
	gcc -lz -g -o lab1b-server -Wextra -Wall lab1b-client.c