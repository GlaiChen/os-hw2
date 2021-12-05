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
   To be portable, it should determine the CPU speed of the machine where it is called. <br/>
   As we use Linux in that assignment, we can locate the CPU in Mhz of the machine at the `/proc/cpuinfo` file as followed: <br/><br/>
   <img src="/images/cpu_mhz.png"> <br/><br/>
   A. In order to calculate the equivalent of cycles in nanoseconds, first we need to calculate the CPU Frequency in GHz. <br/>
      I wrote a function, which use `fopen()`  in READ mode the file `/nproc/cpuinfo` , reads the VM's CPU in MHz and converts it into GHz. <br/><br/>
      ```bash
      static double ghz; 

      static inline int getfreqGHz(double* ghz) {
          FILE* fptr = fopen("/proc/cpuinfo", "r");
          if (fptr == NULL) {
              printf("ERROR: File does not exists");
              return EXIT_FAILURE;
          }

          char temp1, temp2;
          double mhz = 0;                            
          size_t n = 0;
          char* line = NULL;
          while (getline(&line, &n, fptr) > 0) {      
              if (strstr(line, "MHz")) {
                  sscanf(line, "%s %s : %lf", &temp1, &temp2, &mhz);
                  break;
              }
          }
          *ghz = mhz / 1000;
          free(line);
          fclose(fptr);
          return 0;
       }
      ```
      
   B. Now that we have the CPU's frequency in GHz, we can create the simple function `gethosttime()` to calculate the equivalent long long in nanosecond. <br/>
      ```bash
      static inline unsigned long long gethosttime(unsigned long long cycles) {
          return (cycles / ghz);
      }
      ```
      <br/>
2. Using `gethosttime()` , I had to measure how long it takes to execute `getcycles()` . In addition, I had to time the `gettimeofday()`  system call. <br/>
   I should be mindful to do the measurement while minimizing the overhead of doing the measurement. <br/>
   
3. I had to use both `gethosttime()`  and the `gettimeofday()`  system call to time the inner for-loop of the following bit of code: <br/><br/>
   ```bash
      for (i=0; i < 1000; i++) {
          for (j=0; j < 100; j++) {  /* inner loop starts here */
              k = i + j;  
          }                          /* inner loop ends here */
      }
   ```
   <br/>
   The answer should indicate the mean and standard deviation of loop iterations, and explain large variations in the results, if any. <br/>
   Based on the measurements, indicate which timing method is more accurate and explain why. <br/>
<br/>   
## Invidual Part - Understanding of process memory maps
Bla bla
