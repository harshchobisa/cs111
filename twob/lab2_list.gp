#! /usr/local/cs/bin/gnuplot

#graph one
set terminal png
set datafile separator ","

set title "Throughput for the Two Sync Methods"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:28]
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_1.png'
set key left bottom

plot "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'using Mutex Lock' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) title 'using Spin Lock' with linespoints lc rgb 'purple'


#graph two
set title "Completion Time and Lock Waiting Time Averages"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:28]
set ylabel "Time in Seconds"
set logscale y 10
set output 'lab2b_2.png'
set key left top

plot "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) title 'Average Completion Time' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) title 'Average Lock Waiting Time' with linespoints lc rgb 'purple'
    
#graph three
set title "Thread Number compared to Completed Iterations per Lock Type"
unset logscale x
set xrange [0:17]
set xlabel "Number of Threads"
set ylabel "Completed Iterations"
set logscale y 10
set key left top
set output 'lab2b_3.png'

plot "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) with points pointtype 1 lc rgb 'orange' title 'No Locking', \
    "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) with points pointtype 2 lc rgb 'purple' title 'Mutex Lock', \
    "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) with points pointtype 4 lc rgb 'green' title 'Spin Lock', \

#graph four
set title "Throughput for a Partitioned List with Mutex Lock"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_4.png'
set key left bottom
billion = 1000000000

plot "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'orange' title '1 list', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'purple' title '4 lists', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'green' title '8 lists', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'blue' title '16 lists'


#graph five
set title "Throughput for a Partitioned List with Spin Lock"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations per second)"
set logscale y 10
set output 'lab2b_5.png'
set key left bottom

plot "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'orange' title '1 list', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'purple' title '4 lists', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'green' title '8 lists', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) with linespoints lc rgb 'blue' title '16 lists'
