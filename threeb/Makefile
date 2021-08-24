.SILENT:

default:
	chmod +x lab3b.py
	ln -sf lab3b.py lab3b
	chmod +x lab3b

clean:
	rm -f lab3b-505103854.tar.gz lab3b

dist: default
	tar -czvf lab3b-505103854.tar.gz README Makefile lab3b.py
