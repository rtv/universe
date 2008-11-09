
** Instructions **

Read the code and Makefile for some hints on compiling in different
environments.

These are the default parameters, as set in the controller.c file in
this directory.

invert = 0;
angle = 20;
range = 20;
pixels = 9;
population = 100;
seconds = 10000;
updates_per_second = 5;
size = 100;

Your project report must at least describe performance results for the
following parameter settings. In each case, only the
*difference* from the default paramter set is given.

1) Flock

size = 500; 
population = 1000;
seconds = 1000;

2) Worm

size = 500; 
population = 1000;
seconds = 1000;
invert = 1;

3) Disperse

size = 500;
population = 1000;
seconds = 1000;
angle = 360;

The basic performance measure is how much real-world time it takes to
run the simulator for 1000 simulated seconds (so your simulator
performs 1000 seconds * 5 updates per second = 5,000 simulation
steps). You can choose to report a longer or shorter run if necessary.

To measure the real time a process takes, use the "time" command, for
example:

% time ./universe

real    0m32.088s
user    0m30.803s
sys     0m0.366s


You should consider what other performance measures are appropriate to
report. 

You should disable graphics before running experiments! Set the GRAPHICS macro to 0 in universe.c and recompile.

** Scaling **

The interesting part of this project is how your system can scale with
population size. You should repeat the above experiments with a range
of population sizes in order to show how your system scales. You may
choose to only examine scaling for one of the three robot controllers
(flock,worm,disperse). 

In addition to the basic performance score, you should discuss how
scaling affects other dynamic features of your system. For example:

 - how many messages are passed?
 - how is the load balanced across CPUs?
 - where are the bottlenecks that limit performance?


** What to submit **

A gzipped tar file containing your source code and a report in PDF
format, uploaded to the submission server before Sunday 15 April.

** Grading Scheme **

80% distributed system design and execution
    - 20% correctness (e.g. free of deadlocks, properly synchronized)
    - 20% efficiency (e.g. number of messages exchanged)
    - 20% appropriateness of architecture
    - 10% comprehensible code
    - 10% description of the system in the report

20% evaluation design and execution
    - 10% appropriateness of evaluation strategy
    - 10 % presentation of data

The report need not be long, but should contain enough information to
evaluate the system design and performance. Clearly state your design
decisions, your results, and how your decisions influenced the
results.
