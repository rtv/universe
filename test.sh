#!/bin/bash

# ask Bash's built in time command to print only the real time
TIMEFORMAT=%R

rm foo

for P in 10 50 100 200 300 400 500 600 800 1000 1500 2000 2500 3000 3500 4000
 do  
   # record the population size
   echo -e -n "$P\t" >> foo

   # record the run time (time needs a fancy stederr redirect from a subshell)
   { time ./universe -q -p$P  -u100 -d -z0; } 2>> foo
done 

  /usr/local/bin/gnuplot --persist <<\EOF
  set title "Run time vs. Population"
  plot "foo" using 1:2 with linespoints, x**2 / 300000 + 0.1
EOF

