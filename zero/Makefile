.SILENT:


default:
	gcc -g -o lab0 -Wextra -Wall lab0.c

dist: default
	tar -czvf lab0-505103854.tar.gz lab0.c README Makefile backtrace.png breakpoint.png

clean:
	rm -rf lab0 lab0-505103854.tar.gz

check: default
	./lab0 --badarg 2> junk.txt ; \
	if [[ $$? -eq 1 ]]; then echo "PASSED BAD ARGUMENT TEST"; \
	else echo "FAILED BAD ARGUMENT TEST"; \
	fi

	./lab0 --segfault 2> junk.txt ; \
	if [[ $$? -eq 139 ]]; then echo "PASSED SEGFAULT TEST"; \
	else echo "FAILED SEGFAULT TEST"; \
	fi
	
	./lab0 --segfault --catch 2> junk.txt ; \
	if [[ $$? -eq 4 ]]; then echo "PASSED CATCH TEST"; \
	else echo "FAILED CATCH TEST"; \
	fi
	

	./lab0 --input=fakefile.txt 2> junk.txt ; \
	if [[ $$? -eq 2 ]]; then echo "PASSED BAD INPUT FILE TEST"; \
	else echo "FAILED BAD INPUT FILE TEST"; \
	fi
	
	touch cantread.txt;
	chmod -w cantread.txt;

	./lab0 --output=cantread.txt 2> junk.txt ; \
	if [[ $$? -eq 3 ]]; then echo "PASSED BAD OUTPUT FILE TEST"; \
	else echo "FAILED BAD OUTPUT FILE TEST"; \
	fi
	
	rm -f cantread.txt
	rm -f junk.txt
