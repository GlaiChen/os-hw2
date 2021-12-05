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
     static inline double getfreqGHz() {
         FILE* fptr = fopen("/proc/cpuinfo", "r");   
         if (fptr == NULL) {
             printf("ERROR: File PATH /proc/cpuinfo does not exists");
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
       //printf("CPU in MHz: %lf \n", mhz);        //Added only for print-debugging
       //printf("CPU in GHz: %lf \n", (mhz/1000)); //Added only for print-debugging
         free(line);
         fclose(fptr);
         return (mhz / 1000);
       }
      ```
      And the result: <br/><br/>
      <img src="/images/result_mhz_ghz.png">
      <br/><br/>
   B. Now that we have the CPU's frequency in GHz, we can create the simple function `gethosttime()` to calculate the equivalent long long in nanosecond. <br/><br/>
      ```bash
      unsigned long long gethosttime(unsigned long long cycles) {
          double ghz =  getfreqGHz();
        //printf("The equivalent in nanoseconds: %lf \n", cycles / ghz);  //Added only for print-debugging
          return (cycles / ghz);
      }
      ```
      And the result: <br/><br/>
      <img src="/images/equivalent_nanoseconds.png">
      <br/><br/>
2. Using `gethosttime()` , I had to measure how long it takes to execute `getcycles()` . In addition, I had to time the `gettimeofday()`  system call.
   We were asked to be mindful to do the measurement while minimizing the overhead of doing the measurement. <br/><br/>
   ```bash
   static void measure_get_cycles() {
       unsigned long long before = getcycles();
       getcycles();
       unsigned long long after = getcycles();
       unsigned long long diff = (gethosttime(after - before));
       printf("Get cycles took %llu nanosecond\n",diff);
   }
   static void measure_gettimeofday() {
       struct timeval current_time;
       unsigned long long before = getcycles();
       gettimeofday(&current_time, NULL);
       unsigned long long after = getcycles();
       unsigned long long diff = (gethosttime(after - before));
       printf("Get time of day took %llu nanoseconds\n", diff);
   }
   ```
   As you can, I've declared all the variables before, just assigned them when needed. <br/>
   And as for the results: <br/><br/>
   <img src="/images/get_cycles_get_timeofday.png">
   <br/><br/>
3. I had to use both `gethosttime()`  and the `gettimeofday()`  system call to time the inner for-loop of the following bit of code: <br/><br/>
   ```bash
   void calculate_inner_loop() {
       int i, j, k;
       unsigned long long before_cycles, after_cycles;
       suseconds_t before_day, after_day;
       struct timeval current_time;

       for (i=0; i < 1000; i++) {

        before_cycles = getcycles();
        gettimeofday(&current_time, NULL);
        before_day = current_time.tv_usec;

           for (j=0; j < 100; j++) {  /* inner loop starts here */
               k = i + j;  
           }                          /* inner loop ends here */
        after_cycles = getcycles();
        gettimeofday(&current_time, NULL);
        after_day = current_time.tv_usec;
        printf("Inner Loop took %llu nanoseconds and %ld microseconds\n", gethosttime(after_cycles - before_cycles), (after_day - before_day));
       }
   }
   ```
   As you can see, gettimeofday() took much longer because its systemcall is not an assebmley call: <br/><br/>
   <img src="/images/inner_loop.png">
   <br/><br/>
   In addition, the precision of the check is in microseconds and not in nanoseconds. <br/>
   As we noticed in the inner loops, that sometimes getcycles() took more time, it might be interupt by the processor itself. <br/>
   To sum up, the function gethosttime() was much accuarate and much faster. <br/><br/>
   
<br/>   
## Invidual Part - Understanding of process memory maps
Bla bla
