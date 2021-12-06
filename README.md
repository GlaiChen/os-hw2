# "Advanced Topics in Operating Systems" 
 M.Sc. course at the Reichman University <br/>
 Lecturer and guide - Dr. Oren Laadan <br/>
## Intro

This program is a part of our 2nd assignment. <br/>
Check out <a href="https://github.com/GlaiChen/os-hw2/blob/main/assignment-2.md">os-hw2/assignment-2.md</a> for full details of the assignment. <br/>
This program is only the invidual part, and the group part will apear at Branch - ex2 from our repo at the following link: <br/>https://github.com/academy-dt/adv-os-linux/tree/ex2 </br>

## Invidual Part 1 - Timing methodologies
For full answer (which is the actual code) - check out the code at <a href="https://github.com/GlaiChen/os-hw2/blob/main/main.c">os-hw2/main.c </a> <br/>
The main purpose was to write a program that take care of 3 steps:
1. I had to write a function in C called `gethosttime()`  that takes a long long cycles value as an argument and returns the equivalent long long in nanoseconds. <br/>
   To be portable, it should determine the CPU speed of the machine where it is called. <br/>
   As we use Linux in that assignment, we can locate the CPU in MHz of the machine at the `/proc/cpuinfo` file as followed: <br/><br/>
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
   I've declared all the variables before, just assigned them when needed. <br/>
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
   As you can see, `gettimeofday()` took much longer because its systemcall is not an assebmley call. <br/>
   Moreover, there are higher chances of a context switch in `gettimeofday()`: <br/><br/>
   <img src="/images/inner_loop.png">
   <br/><br/>
   In addition, the precision of the check is in microseconds and not in nanoseconds. <br/>
   As we noticed in the inner loops, that sometimes `getcycles()` took more time, it might be interrupt by the processor itself. <br/>
   To sum up, the function `gethosttime()` was much accuarate and much faster. <br/><br/>
## Invidual Part 2 - Understanding of process memory maps
1. Q: Explain the page table mappings of each region of `cat /proc/self/maps`  by stating precisely what each region is used for (e.g. [stack] is the stack of the process).<br/>
   A: I have copied the `/proc/self/maps`  to a new txt file and edited it, so I can easily present it here with the full detailed answer: <br/><br/>
   ```bash
   ############################################################################################################################
            address         perms  offset   dev  inode                       pathname
   ############################################################################################################################

   #A Private process with Read only permissions, which was not mapped from a file (= 00000000)
   56455c362000-56455c364000 r--p 00000000 08:01 1566                       /usr/bin/cat
   #
   56455c364000-56455c369000 r-xp 00002000 08:01 1566                       /usr/bin/cat
   #
   56455c369000-56455c36c000 r--p 00007000 08:01 1566                       /usr/bin/cat
   #
   56455c36c000-56455c36d000 r--p 00009000 08:01 1566                       /usr/bin/cat
   #
   56455c36d000-56455c36e000 rw-p 0000a000 08:01 1566                       /usr/bin/cat
   #
   56455cdc3000-56455cde4000 rw-p 00000000 00:00 0                          [heap]
   #
   7f788814f000-7f7888171000 rw-p 00000000 00:00 0
   #
   7f7888171000-7f78881a3000 r--p 00000000 08:01 5978                       /usr/lib/locale/C.UTF-8/LC_CTYPE
   #
   7f78881a3000-7f78881a4000 r--p 00000000 08:01 5983                       /usr/lib/locale/C.UTF-8/LC_NUMERIC
   #
   7f78881a4000-7f78881a5000 r--p 00000000 08:01 5986                       /usr/lib/locale/C.UTF-8/LC_TIME
   #
   7f78881a5000-7f7888318000 r--p 00000000 08:01 5977                       /usr/lib/locale/C.UTF-8/LC_COLLATE
   #
   7f7888318000-7f7888319000 r--p 00000000 08:01 5981                       /usr/lib/locale/C.UTF-8/LC_MONETARY
   #
   #
   7f7888319000-7f788831a000 r--p 00000000 08:01 5975                       /usr/lib/locale/C.UTF-8/LC_MESSAGES/SYS_LC_MESSAGES

   #
   7f788831a000-7f788831b000 r--p 00000000 08:01 5984                       /usr/lib/locale/C.UTF-8/LC_PAPER

   #
   7f788831b000-7f788831c000 r--p 00000000 08:01 5982                       /usr/lib/locale/C.UTF-8/LC_NAME

   #
   7f788831c000-7f788831d000 r--p 00000000 08:01 5976                       /usr/lib/locale/C.UTF-8/LC_ADDRESS

   #
   7f788831d000-7f788831e000 r--p 00000000 08:01 5985                       /usr/lib/locale/C.UTF-8/LC_TELEPHONE

   #
   7f788831e000-7f7888604000 r--p 00000000 08:01 5972                       /usr/lib/locale/locale-archive
   #
   7f7888604000-7f7888629000 r--p 00000000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   7f7888629000-7f78887a1000 r-xp 00025000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   7f78887a1000-7f78887eb000 r--p 0019d000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   7f78887eb000-7f78887ec000 ---p 001e7000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   7f78887ec000-7f78887ef000 r--p 001e7000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   7f78887ef000-7f78887f2000 rw-p 001ea000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so

   # Anonymous Read/Write segment which may be used as a backing store.
   # Read/Write permissions and Private.
   7f78887f2000-7f78887f8000 rw-p 00000000 00:00 0
   #
   7f78887f8000-7f78887f9000 r--p 00000000 08:01 5980                       /usr/lib/locale/C.UTF-8/LC_MEASUREMENT
   #
   7f78887f9000-7f7888800000 r--s 00000000 08:01 3756                       /usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache

   #
   7f7888800000-7f7888801000 r--p 00000000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   7f7888801000-7f7888824000 r-xp 00001000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   7f7888824000-7f788882c000 r--p 00024000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so

   #
   7f788882c000-7f788882d000 r--p 00000000 08:01 5979                       /usr/lib/locale/C.UTF-8/LC_IDENTIFICATION

   #
   7f788882d000-7f788882e000 r--p 0002c000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   7f788882e000-7f788882f000 rw-p 0002d000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   
   # Anonymous Read/Write segment which may be used as a backing store.
   # Read/Write permissions and Private.
   7f788882f000-7f7888830000 rw-p 00000000 00:00 0

   # This is the main thread’s stack segment which enfolds the local variables.
   # Read/Write and private permissions.
   7fff39ae0000-7fff39b01000 rw-p 00000000 00:00 0                          [stack]

   # Mapping for the [vsyscall]. Variables from the kernel address space are which mapped to the user-space addresses.
   # It is Read-only and private.
   7fff39be0000-7fff39be3000 r--p 00000000 00:00 0                          [vvar]

   # vDSO - Virtual Dynamic Shared Objects memory region (will be explained in Q4).
   # It is the nowdays used mechanism.
   # Read and Executable only and private.
   7fff39be3000-7fff39be4000 r-xp 00000000 00:00 0                          [vdso]

   # vsyscall - Virtual system call memory region (will be explained in Q4)
   # It is the old mechanism, used when the kernel does not have vDSO support or need compatibility
   # Executable only and private.
   ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
   ```
2. Q: Explore the page table mappings of systemd-journald using the command `cat /proc/$(pidof systemd-journald)/maps` .  <br/>
      Explain the regions types that you have not seen in (Q1).. <br/>
3. Q: Do `cat /proc/self/maps`  multiple times on your system (laptop or VM). What do you notice regarding the page table mapping? Why? <br/>
   A: When I did `cat /proc/self/maps` multiple times on my system (the given VM from Prof. Laadan), I have noticed that the addresses change each time. <br/>
      The reason is quite simple, the OS itself is randomizing the addresses of all libraries in the system. <br/>
      The reason for the randomization is SECURITY. The more the system can randomize addresses, <br/> the more the adversary is having hard times to find the libraries he's        searching for implementing diverse attacks.
4. Q: Notice the [vdso] mapping that is common to all processe. Read the man page (`man vdso`), and explain its purpose and the problems it aims to solve. <br/>
   A: [vDSO], which its full name is "Virtual Dynamic Shared Object", as defined, is a small shared library that the kernel automatically maps into the address
       space of all user space applications. <br/>
       It's purpose is being an alternative to the cycle-expensive system call interface that the kernel provides us (`vsyscall`). <br/>
       As we learned in class and practiced in this assignment, we've noticed that in order to accomplish a system call, the kernel must switch between memory contexts, the user space and the kernel space, <br/> due to secturity problems (among the rest). When calling a simple syscalls, the system is trying to avoid the overhead of context switching into the kernel, and the result is a `vsyscall` which takes longer to execute. <br/>
       We can have for example the `gettimeofday()`; This simple syscall provide no threat to the kernel and there's no real need to do the context switch between the user space and the kernel space. <br/>
       The `vDSO` offers the same functionality as the vsyscall, while bridging of the vsyscall gaps. <br/>
       The `vDSO` maps memory pages into each process into a shared object form, which helps the system so all userspace applications that dynamically link to glibc will use the vDSO automatically. <br/> It exposes some kernel functionalities at user space in a safe manner.
       This has been introduced to solve the security threats caused by the `vsyscall`. <br/>
       Another problem the `vDSO` aim to solve. is the limit that the `vsyscall` produce - up to 4 system calls.<br/>
       To sum up, the `vDSO` is dynamically allocated and aim to solve the security concerns such as fixed addresses and it can have more than 4 system calls.<br/>
