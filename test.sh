#!/bin/bash

# ask Bash's built in time command to print only the real time
TIMEFORMAT=%R

for P in 10 50 100 200 300 400 500 600 800 1000 1200
 do  
   # record the population size
   echo -e -n "$P\t" >> foo

   # record the run time (time needs a fancy stederr redirect from a subshell)
   { time ./universe -p$P -d -q -u200 -z0; } 2>> foo
done 

  gnuplot --persist <<\EOF
  set title "Run time vs. Population"
  plot "foo" using 1:2 with linespoints, x**2 / 300000 + 0.1
EOF

