#!/bin/bash

# ask Bash's built in time command to print only the real time
TIMEFORMAT=%R

WIN=200
SLP=0

rm eval1 eval2 eval3

for P in 10 100 200 300 400 500 600 800 1000 2000 3000 4000
 do  
   # record the population size
   echo -e -n "$P\t" >> eval1
   echo -e -n "$P\t" >> eval2
   echo -e -n "$P\t" >> eval3

   # record the run time (time needs a fancy stederr redirect from a subshell)
   { time ./universe -q -w$WIN -p$P  -u100 -d -z$SLP -s1.0; } 2>> eval1
   { time ./universe -q -w$WIN -p$P  -u100 -d -z$SLP -s2.0; } 2>> eval2
   { time ./universe -q -w$WIN -p$P  -u100 -d -z$SLP -s5.0; } 2>> eval3  
done 

  /usr/local/bin/gnuplot --persist <<\EOF
  set title "Run time vs. Population"
  plot "eval1" using 1:2 with linespoints title "Size 1", \
       "eval2" using 1:2 with linespoints title "Size 2", \
       "eval3" using 1:2 with linespoints title "Size 5"
EOF

