default:
	gcc -g -lrt -pthread -lprofiler -Wall -Wextra SortedList.c -o lab2_list lab2_list.c

clean:
	rm -f *.o *.tar.gz lab2_list

tests: default
	chmod +x tests.sh
	./tests.sh
	
dist: graphs 
	tar -czvf lab2b-505103854.tar.gz tests.sh lab2_list.c SortedList.h \
	SortedList.c lab2b_list.csv lab2_list.gp Makefile README lab2b_1.png \
	lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png profile.out

graphs: tests
	chmod +x lab2_list.gp
	./lab2_list.gp

profile:
	rm -rf profile.out profile.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./lab2b.gperf ./lab2_list \
	--threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./lab2b.gperf > profile.out
	pprof --list=thread_function_for_list ./lab2_list ./lab2b.gperf >> profile.out
	rm -rf ./lab2b.gperf