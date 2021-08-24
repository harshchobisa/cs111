.SILENT:


default:
	gcc -g -o lab1b-client -Wextra -Wall lab1b-client.c
	gcc -g -o lab1b-server -Wextra -Wall lab1b-server.c


dist: default
	tar -czvf lab1a-505103854.tar.gz lab1a.c README Makefile 

clean:
	rm -rf lab1a lab1a-505103854.tar.gz

