# "Advanced Topics in Operating Systems" 
 M.Sc. course at the Reichman University <br/>
 Lecturer and guide - Dr. Oren Laadan <br/>
## Intro

This program is a part of our 2nd assignment. <br/>
Check out <a href="https://github.com/GlaiChen/os-hw2/blob/main/assignment-2.md">os-hw2/assignment-2.md</a> for full details of the assignment. <br/>
This program is only the invidual part, and the group part will apear at Branch - ex2 from our repo at the following link: <br/>https://github.com/academy-dt/adv-os-linux/tree/ex2 </br>

## Invidual Part - Timing methodologies
For full answer (which is the actual code) - check out the code at <a href="https://github.com/GlaiChen/os-hw2/blob/main/main.c">os-hw2/main.c </a> <br/>
The main purpose was to write a program that take care of 3 steps:
1. I had to write a function in C called `gethosttime()`  that takes a long long cycles value as an argument and returns the equivalent long long in nanoseconds. <br/>
   To be portable, it should determine the CPU speed of the machine where it is called (In Linux, we can use the data in /proc/cpuinfo for this).
2. Using `gethosttime()` , I had to measure how long it takes to execute `getcycles()` . In addition, I had to time the `gettimeofday()`  system call. 
   I should be mindful to do the measurement while minimizing the overhead of doing the measurement.
3. I had to use both `gethosttime()`  and the `gettimeofday()`  system call to time the inner for-loop of the following bit of code:
```bash
   for (i=0; i < 1000; i++) {
       for (j=0; j < 100; j++) {  /* inner loop starts here */
           k = i + j;  
       }                          /* inner loop ends here */
   }
```
   The answer should indicate the mean and standard deviation of loop iterations, and explain large variations in the results, if any. <br/>
   Based on the measurements, indicate which timing method is more accurate and explain why. <br/>
<br/>   
## Invidual Part - Understanding of process memory maps
Bla bla
