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

   # A Private BINARY ELF with Read only permissions, which was not mapped from a file (= 00000000). It's the .rodata segment.
   56455c362000-56455c364000 r--p 00000000 08:01 1566                       /usr/bin/cat
   
   # Another Private BINARY ELF with Read && Executable only permissions, which mapped from the upper line (offset 2000). It's the .text segment.
   56455c364000-56455c369000 r-xp 00002000 08:01 1566                       /usr/bin/cat
   
   # Another Private process with Read only permissions, which mapped from the first line (offset 7000). It's .rodata segment.
   56455c369000-56455c36c000 r--p 00007000 08:01 1566                       /usr/bin/cat
   
   # Another Private process with Read only permissions, which is .rodata segment.
   56455c36c000-56455c36d000 r--p 00009000 08:01 1566                       /usr/bin/cat
   
   # Another Private process with Read/Write permissions, which is .data segment.
   56455c36d000-56455c36e000 rw-p 0000a000 08:01 1566                       /usr/bin/cat
   
   # The location of the Heap memory segment, with Read/Write permissions. This is where the memory used for dynamic allocations.
   56455cdc3000-56455cde4000 rw-p 00000000 00:00 0                          [heap]
   
   # Anonymous Read/Write segment which may be used as a backing store.
   # Read/Write permissions and Private.
   7f788814f000-7f7888171000 rw-p 00000000 00:00 0
  
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_CTYPE is character classification and case conversion, mapped as Read-only && Private permissions. .rodata segment.
   7f7888171000-7f78881a3000 r--p 00000000 08:01 5978                       /usr/lib/locale/C.UTF-8/LC_CTYPE
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_NUMERIC is numeric formatting, mapped as Read-only && Private permissions. .rodata segment.
   7f78881a3000-7f78881a4000 r--p 00000000 08:01 5983                       /usr/lib/locale/C.UTF-8/LC_NUMERIC
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_TIME is date and time formats, mapped as Read-only && Private permissions. .rodata segment.
   7f78881a4000-7f78881a5000 r--p 00000000 08:01 5986                       /usr/lib/locale/C.UTF-8/LC_TIME
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_COLLATE is collation (sort) order, mapped as Read-only && Private permissions. .rodata segment.
   7f78881a5000-7f7888318000 r--p 00000000 08:01 5977                       /usr/lib/locale/C.UTF-8/LC_COLLATE
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_MONETARY is monetary formatting, mapped as Read-only && Private permissions. .rodata segment.
   7f7888318000-7f7888319000 r--p 00000000 08:01 5981                       /usr/lib/locale/C.UTF-8/LC_MONETARY
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # SYS_LC_MESSAGES is format of interactive words and responses, mapped as Read-only && Private permissions. .rodata segment.
   7f7888319000-7f788831a000 r--p 00000000 08:01 5975                       /usr/lib/locale/C.UTF-8/LC_MESSAGES/SYS_LC_MESSAGES
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_PAPER is default paper size for region, mapped as Read-only && Private permissions. .rodata segment.
   7f788831a000-7f788831b000 r--p 00000000 08:01 5984                       /usr/lib/locale/C.UTF-8/LC_PAPER

   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_NAME is name formats, mapped as Read-only && Private permissions. .rodata segment.
   7f788831b000-7f788831c000 r--p 00000000 08:01 5982                       /usr/lib/locale/C.UTF-8/LC_NAME

   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_ADDRESS is convention used for formatting of street or postal addresses, mapped as Read-only && Private permissions. .rodata segment.
   7f788831c000-7f788831d000 r--p 00000000 08:01 5976                       /usr/lib/locale/C.UTF-8/LC_ADDRESS
   
   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_TELEPHONE is conventions used for representation of telephone numbers, mapped as Read-only && Private permissions. .rodata segment.
   7f788831d000-7f788831e000 r--p 00000000 08:01 5985                       /usr/lib/locale/C.UTF-8/LC_TELEPHONE

   # A locale archive is a Read-only && Private memory-mapped file which contains all the system-provided localesץ
   # It is used by all localized programs when the environment variable LOCPATH is not set. This is .rodata segment
   7f788831e000-7f7888604000 r--p 00000000 08:01 5972                       /usr/lib/locale/locale-archive
   
   # The glibc library mapped as Read-only && Private. This is its .rodata segment
   7f7888604000-7f7888629000 r--p 00000000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   
   # The glibc library mapped as Read-only && Executable && Private. This is its .text segment
   7f7888629000-7f78887a1000 r-xp 00025000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   
   # The glibc library mapped as Read-only && Private. This is its .rodata segment
   7f78887a1000-7f78887eb000 r--p 0019d000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   
   # The glibc library mapped without any permissions, which means it is a guard page, which  causes a segmentation fault upon any access. 
   # As a result, any attempt by an attacker to overwrite adjacent memory in the course of exploiting a buffer overflow causes the vulnerable program to terminate rather than continue execution of the attacker-supplied code
   7f78887eb000-7f78887ec000 ---p 001e7000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   
   # The glibc library mapped as Read-only && Private. This is its .rodata segment
   7f78887ec000-7f78887ef000 r--p 001e7000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so
   
   # The glibc library mapped as Read/Write && Private. This is its .data segment
   7f78887ef000-7f78887f2000 rw-p 001ea000 08:01 3447                       /usr/lib/x86_64-linux-gnu/libc-2.31.so

   # Anonymous Read/Write segment which may be used as a backing store.
   # Read/Write permissions and Private.
   7f78887f2000-7f78887f8000 rw-p 00000000 00:00 0

   # A part of Locales (set of environment variables that are used to determine how to display data and time), symbols encoding (for example, how to display cyrillic symbols), default files order when one executing the ls command, and so on.
   # LC_MEASUREMENT is default measurement system used within the region, mapped as Read-only && Private permissions. .rodata segment.
   7f78887f8000-7f78887f9000 r--p 00000000 08:01 5980                       /usr/lib/locale/C.UTF-8/LC_MEASUREMENT
  
   # Usual system gconv module configuration cache, mapped as Read-only && Shared, this is its .rodata.
   7f78887f9000-7f7888800000 r--s 00000000 08:01 3756                       /usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache

   # Dynamic Linker/Loader mapped as Read-only && private permissions, this is its .rodata segment.
   7f7888800000-7f7888801000 r--p 00000000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   
   # Dynamic Linker/Loader mapped as Read && Executable && Private permissions, this is its .data segment.
   7f7888801000-7f7888824000 r-xp 00001000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   
   # Dynamic Linker/Loader mapped as Read-only && private permissions, this is its .rodata segment.
   7f7888824000-7f788882c000 r--p 00024000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so

   # A part of Locales (set of environment variables that are used to determine how to display data and time).
   # LC_IDENTIFICATION is metadata about the locale information, mapped as Read-only && Private permissions. .rodata segment.
   7f788882c000-7f788882d000 r--p 00000000 08:01 5979                       /usr/lib/locale/C.UTF-8/LC_IDENTIFICATION

   # Dynamic Linker/Loader mapped as Read-only && private permissions, this is its .rodata segment.
   7f788882d000-7f788882e000 r--p 0002c000 08:01 3443                       /usr/lib/x86_64-linux-gnu/ld-2.31.so
   
   # Dynamic Linker/Loader mapped as Read/Write && private permissions, this is its .data segment.
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
      Explain the regions types that you have not seen in (Q1). <br/>
   A: I have copied the `cat /proc/$(pidof systemd-journald)/maps`  to a new txt file and edited it, so I can easily present it here with the full detailed answer: <br/><br/>
   ```bash
   ############################################################################################################################
            address         perms  offset   dev  inode                       pathname
   ############################################################################################################################
   55c9f471b000-55c9f4721000 r--p 00000000 08:01 3308                       /usr/lib/systemd/systemd-journald
   55c9f4721000-55c9f4739000 r-xp 00006000 08:01 3308                       /usr/lib/systemd/systemd-journald
   55c9f4739000-55c9f4741000 r--p 0001e000 08:01 3308                       /usr/lib/systemd/systemd-journald
   55c9f4741000-55c9f4743000 r--p 00025000 08:01 3308                       /usr/lib/systemd/systemd-journald
   55c9f4743000-55c9f4744000 rw-p 00027000 08:01 3308                       /usr/lib/systemd/systemd-journald
   
   7f617a38f000-7f617ab8f000 rw-s 00000000 08:01 258198                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/user-1000.journal
   7f617ab8f000-7f617b38f000 rw-s 00000000 08:01 258056                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/system.journal
   
   /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/system@0d5c652224374643a8af3631f39c4c62-0000000000000001-0005d27bd51b4b4d.journal
   7f617c390000-7f617cb90000 rw-s 00000000 08:01 258191                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/user-1000@2786f561e68b4fab9eaccae4ea73a899- 0000000000000386-0005d26aee3419f5.journal

   7f617daf5000-7f617daf6000 rw-s 00000000 08:01 258198                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/user-1000.journal
   7f617daf6000-7f617daf7000 rw-s 00000000 08:01 258192                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/system@0d5c652224374643a8af3631f39c4c62-0000000000000001-0005d27bd51b4b4d.journal
   7f617daf7000-7f617daf8000 rw-s 00000000 08:01 258056                     /var/log/journal/daa5f54b52c44b9886fdaa70812de1c5/system.journal
   ```
   Moreover, we can see many more objects from the same regions from Q1: <br/><br/>
   ```bash
   ############################################################################################################################
            address         perms  offset   dev  inode                       pathname
   ############################################################################################################################
    7f617cb97000-7f617cb99000 r--p 00000000 08:01 3467                       /usr/lib/x86_64-linux-gnu/libcap-ng.so.0.0.0
   7f617cb99000-7f617cb9c000 r-xp 00002000 08:01 3467                       /usr/lib/x86_64-linux-gnu/libcap-ng.so.0.0.0
   7f617cb9c000-7f617cb9d000 r--p 00005000 08:01 3467                       /usr/lib/x86_64-linux-gnu/libcap-ng.so.0.0.0
   7f617cb9d000-7f617cb9e000 r--p 00005000 08:01 3467                       /usr/lib/x86_64-linux-gnu/libcap-ng.so.0.0.0
   7f617cb9e000-7f617cb9f000 rw-p 00006000 08:01 3467                       /usr/lib/x86_64-linux-gnu/libcap-ng.so.0.0.0
   7f617cb9f000-7f617cbae000 r--p 00000000 08:01 3449                       /usr/lib/x86_64-linux-gnu/libm-2.31.so
   7f617cbae000-7f617cc55000 r-xp 0000f000 08:01 3449                       /usr/lib/x86_64-linux-gnu/libm-2.31.so
   7f617cc55000-7f617ccec000 r--p 000b6000 08:01 3449                       /usr/lib/x86_64-linux-gnu/libm-2.31.so
   7f617ccec000-7f617cced000 r--p 0014c000 08:01 3449                       /usr/lib/x86_64-linux-gnu/libm-2.31.so
   7f617cced000-7f617ccee000 rw-p 0014d000 08:01 3449                       /usr/lib/x86_64-linux-gnu/libm-2.31.so
   7f617ccee000-7f617ccf3000 r--p 00000000 08:01 3863                       /usr/lib/x86_64-linux-gnu/libudev.so.1.6.17
   7f617ccf3000-7f617cd0f000 r-xp 00005000 08:01 3863                       /usr/lib/x86_64-linux-gnu/libudev.so.1.6.17
   7f617cd0f000-7f617cd19000 r--p 00021000 08:01 3863                       /usr/lib/x86_64-linux-gnu/libudev.so.1.6.17
   7f617cd19000-7f617cd1a000 r--p 0002a000 08:01 3863                       /usr/lib/x86_64-linux-gnu/libudev.so.1.6.17
   7f617cd1a000-7f617cd1b000 rw-p 0002b000 08:01 3863                       /usr/lib/x86_64-linux-gnu/libudev.so.1.6.17
   7f617cd1b000-7f617cd1e000 r--p 00000000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd1e000-7f617cd26000 r-xp 00003000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd26000-7f617cd3a000 r--p 0000b000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd3a000-7f617cd3b000 ---p 0001f000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd3b000-7f617cd3c000 r--p 0001f000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd3c000-7f617cd3d000 rw-p 00020000 08:01 3502                       /usr/lib/x86_64-linux-gnu/libaudit.so.1.0.0
   7f617cd49000-7f617cd59000 r--p 00000000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617cd59000-7f617cd8f000 r-xp 00010000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617cd8f000-7f617cec6000 r--p 00046000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617cec6000-7f617cec7000 ---p 0017d000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617cec7000-7f617ceca000 r--p 0017d000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617ceca000-7f617cecb000 rw-p 00180000 08:01 3844                       /usr/lib/x86_64-linux-gnu/libunistring.so.2.1.0
   7f617cecb000-7f617cecf000 r--p 00000000 08:01 3766                       /usr/lib/x86_64-linux-gnu/libgpg-error.so.0.28.0
   7f617cecf000-7f617cee2000 r-xp 00004000 08:01 3766                       /usr/lib/x86_64-linux-gnu/libgpg-error.so.0.28.0
   7f617cee2000-7f617ceec000 r--p 00017000 08:01 3766                       /usr/lib/x86_64-linux-gnu/libgpg-error.so.0.28.0
   7f617ceec000-7f617ceed000 r--p 00020000 08:01 3766                       /usr/lib/x86_64-linux-gnu/libgpg-error.so.0.28.0
   7f617ceed000-7f617ceee000 rw-p 00021000 08:01 3766                       /usr/lib/x86_64-linux-gnu/libgpg-error.so.0.28.0
   7f617ceee000-7f617cef2000 r--p 00000000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617cef2000-7f617cefa000 r-xp 00004000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617cefa000-7f617cefd000 r--p 0000c000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617cefd000-7f617cefe000 ---p 0000f000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617cefe000-7f617ceff000 r--p 0000f000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617ceff000-7f617cf00000 rw-p 00010000 08:01 3896                       /usr/lib/x86_64-linux-gnu/libjson-c.so.4.0.0
   7f617cf00000-7f617cf01000 r--p 00000000 08:01 3480                       /usr/lib/x86_64-linux-gnu/libargon2.so.1
   7f617cf01000-7f617cf06000 r-xp 00001000 08:01 3480                       /usr/lib/x86_64-linux-gnu/libargon2.so.1
   7f617cf06000-7f617cf08000 r--p 00006000 08:01 3480                       /usr/lib/x86_64-linux-gnu/libargon2.so.1
   7f617cf08000-7f617cf09000 r--p 00007000 08:01 3480                       /usr/lib/x86_64-linux-gnu/libargon2.so.1
   7f617cf09000-7f617cf0a000 rw-p 00008000 08:01 3480                       /usr/lib/x86_64-linux-gnu/libargon2.so.1
   7f617cf0a000-7f617cf82000 r--p 00000000 08:01 3898                       /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
   7f617cf82000-7f617d11d000 r-xp 00078000 08:01 3898                       /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
   7f617d11d000-7f617d1ae000 r--p 00213000 08:01 3898                       /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
   7f617d1ae000-7f617d1da000 r--p 002a3000 08:01 3898                       /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
   7f617d1da000-7f617d1dc000 rw-p 002cf000 08:01 3898                       /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
   7f617d1e2000-7f617d1ec000 r--p 00000000 08:01 3492                       /usr/lib/x86_64-linux-gnu/libdevmapper.so.1.02.1
   7f617d1ec000-7f617d234000 r-xp 0000a000 08:01 3492                       /usr/lib/x86_64-linux-gnu/libdevmapper.so.1.02.1
   7f617d234000-7f617d248000 r--p 00052000 08:01 3492                       /usr/lib/x86_64-linux-gnu/libdevmapper.so.1.02.1
   7f617d248000-7f617d249000 r--p 00065000 08:01 3492                       /usr/lib/x86_64-linux-gnu/libdevmapper.so.1.02.1
   7f617d249000-7f617d24c000 rw-p 00066000 08:01 3492                       /usr/lib/x86_64-linux-gnu/libdevmapper.so.1.02.1
   7f617d24d000-7f617d24f000 r--p 00000000 08:01 3859                       /usr/lib/x86_64-linux-gnu/libuuid.so.1.3.0
   7f617d24f000-7f617d253000 r-xp 00002000 08:01 3859                       /usr/lib/x86_64-linux-gnu/libuuid.so.1.3.0
   7f617d253000-7f617d254000 r--p 00006000 08:01 3859                       /usr/lib/x86_64-linux-gnu/libuuid.so.1.3.0
   7f617d254000-7f617d255000 r--p 00006000 08:01 3859                       /usr/lib/x86_64-linux-gnu/libuuid.so.1.3.0
   7f617d255000-7f617d256000 rw-p 00007000 08:01 3859                       /usr/lib/x86_64-linux-gnu/libuuid.so.1.3.0
   7f617d256000-7f617d257000 r--p 00000000 08:01 3448                       /usr/lib/x86_64-linux-gnu/libdl-2.31.so
   7f617d257000-7f617d259000 r-xp 00001000 08:01 3448                       /usr/lib/x86_64-linux-gnu/libdl-2.31.so
   7f617d259000-7f617d25a000 r--p 00003000 08:01 3448                       /usr/lib/x86_64-linux-gnu/libdl-2.31.so
   7f617d25a000-7f617d25b000 r--p 00003000 08:01 3448                       /usr/lib/x86_64-linux-gnu/libdl-2.31.so
   7f617d25b000-7f617d25c000 rw-p 00004000 08:01 3448                       /usr/lib/x86_64-linux-gnu/libdl-2.31.so
   7f617d25c000-7f617d25e000 r--p 00000000 08:01 3834                       /usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.9.0
   7f617d25e000-7f617d2c2000 r-xp 00002000 08:01 3834                       /usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.9.0
   7f617d2c2000-7f617d2ea000 r--p 00066000 08:01 3834                       /usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.9.0
   7f617d2ea000-7f617d2eb000 r--p 0008d000 08:01 3834                       /usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.9.0
   7f617d2eb000-7f617d2ec000 rw-p 0008e000 08:01 3834                       /usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.9.0
   7f617d2ec000-7f617d2f3000 r--p 00000000 08:01 3460                       /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
   7f617d2f3000-7f617d304000 r-xp 00007000 08:01 3460                       /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
   7f617d304000-7f617d309000 r--p 00018000 08:01 3460                       /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
   7f617d309000-7f617d30a000 r--p 0001c000 08:01 3460                       /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
   7f617d30a000-7f617d30b000 rw-p 0001d000 08:01 3460                       /usr/lib/x86_64-linux-gnu/libpthread-2.31.so
   7f617d311000-7f617d314000 r--p 00000000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d314000-7f617d32c000 r-xp 00003000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d32c000-7f617d337000 r--p 0001b000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d337000-7f617d338000 ---p 00026000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d338000-7f617d339000 r--p 00026000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d339000-7f617d33a000 rw-p 00027000 08:01 3768                       /usr/lib/x86_64-linux-gnu/liblzma.so.5.2.4
   7f617d33a000-7f617d33c000 r--p 00000000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d33c000-7f617d34b000 r-xp 00002000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d34b000-7f617d359000 r--p 00011000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d359000-7f617d35a000 ---p 0001f000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d35a000-7f617d35b000 r--p 0001f000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d35b000-7f617d35c000 rw-p 00020000 08:01 3832                       /usr/lib/x86_64-linux-gnu/libseccomp.so.2.5.1
   7f617d35c000-7f617d35f000 r--p 00000000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d35f000-7f617d363000 r-xp 00003000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d363000-7f617d364000 r--p 00007000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d364000-7f617d365000 ---p 00008000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d365000-7f617d366000 r--p 00008000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d366000-7f617d367000 rw-p 00009000 08:01 3464                       /usr/lib/x86_64-linux-gnu/librt-2.31.so
   7f617d367000-7f617d36a000 r--p 00000000 08:01 3866                       /usr/lib/x86_64-linux-gnu/libpam.so.0.84.2
   7f617d36a000-7f617d373000 r-xp 00003000 08:01 3866                       /usr/lib/x86_64-linux-gnu/libpam.so.0.84.2
   7f617d373000-7f617d377000 r--p 0000c000 08:01 3866                       /usr/lib/x86_64-linux-gnu/libpam.so.0.84.2
   7f617d377000-7f617d378000 r--p 0000f000 08:01 3866                       /usr/lib/x86_64-linux-gnu/libpam.so.0.84.2
   7f617d378000-7f617d379000 rw-p 00010000 08:01 3866                       /usr/lib/x86_64-linux-gnu/libpam.so.0.84.2
   7f617d379000-7f617d384000 r--p 00000000 08:01 3908                       /usr/lib/x86_64-linux-gnu/libmount.so.1.1.0
   7f617d384000-7f617d3c2000 r-xp 0000b000 08:01 3908                       /usr/lib/x86_64-linux-gnu/libmount.so.1.1.0
   7f617d3c2000-7f617d3d5000 r--p 00049000 08:01 3908                       /usr/lib/x86_64-linux-gnu/libmount.so.1.1.0
   7f617d3d5000-7f617d3d8000 r--p 0005b000 08:01 3908                       /usr/lib/x86_64-linux-gnu/libmount.so.1.1.0
   7f617d3d8000-7f617d3d9000 rw-p 0005e000 08:01 3908                       /usr/lib/x86_64-linux-gnu/libmount.so.1.1.0
   7f617d3d9000-7f617d3db000 r--p 00000000 08:01 3481                       /usr/lib/x86_64-linux-gnu/liblz4.so.1.9.2
   7f617d3db000-7f617d3f5000 r-xp 00002000 08:01 3481                       /usr/lib/x86_64-linux-gnu/liblz4.so.1.9.2
   7f617d3f5000-7f617d3f8000 r--p 0001c000 08:01 3481                       /usr/lib/x86_64-linux-gnu/liblz4.so.1.9.2
   7f617d3f8000-7f617d3f9000 r--p 0001e000 08:01 3481                       /usr/lib/x86_64-linux-gnu/liblz4.so.1.9.2
   7f617d3f9000-7f617d3fa000 rw-p 0001f000 08:01 3481                       /usr/lib/x86_64-linux-gnu/liblz4.so.1.9.2
   7f617d3fc000-7f617d3ff000 r--p 00000000 08:01 3772                       /usr/lib/x86_64-linux-gnu/libkmod.so.2.3.5
   7f617d3ff000-7f617d40f000 r-xp 00003000 08:01 3772                       /usr/lib/x86_64-linux-gnu/libkmod.so.2.3.5
   7f617d40f000-7f617d415000 r--p 00013000 08:01 3772                       /usr/lib/x86_64-linux-gnu/libkmod.so.2.3.5
   7f617d415000-7f617d416000 r--p 00018000 08:01 3772                       /usr/lib/x86_64-linux-gnu/libkmod.so.2.3.5
   7f617d416000-7f617d417000 rw-p 00019000 08:01 3772                       /usr/lib/x86_64-linux-gnu/libkmod.so.2.3.5
   7f617d417000-7f617d419000 r--p 00000000 08:01 3764                       /usr/lib/x86_64-linux-gnu/libip4tc.so.2.0.0
   7f617d419000-7f617d41d000 r-xp 00002000 08:01 3764                       /usr/lib/x86_64-linux-gnu/libip4tc.so.2.0.0
   7f617d41d000-7f617d41f000 r--p 00006000 08:01 3764                       /usr/lib/x86_64-linux-gnu/libip4tc.so.2.0.0
   7f617d41f000-7f617d420000 r--p 00007000 08:01 3764                       /usr/lib/x86_64-linux-gnu/libip4tc.so.2.0.0
   7f617d420000-7f617d421000 rw-p 00008000 08:01 3764                       /usr/lib/x86_64-linux-gnu/libip4tc.so.2.0.0
   7f617d421000-7f617d423000 r--p 00000000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d423000-7f617d428000 r-xp 00002000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d428000-7f617d43f000 r--p 00007000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d43f000-7f617d440000 ---p 0001e000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d440000-7f617d441000 r--p 0001e000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d441000-7f617d442000 rw-p 0001f000 08:01 3503                       /usr/lib/x86_64-linux-gnu/libidn2.so.0.3.6
   7f617d442000-7f617d44e000 r--p 00000000 08:01 3861                       /usr/lib/x86_64-linux-gnu/libgcrypt.so.20.2.5
   7f617d44e000-7f617d51c000 r-xp 0000c000 08:01 3861                       /usr/lib/x86_64-linux-gnu/libgcrypt.so.20.2.5
   7f617d51c000-7f617d559000 r--p 000da000 08:01 3861                       /usr/lib/x86_64-linux-gnu/libgcrypt.so.20.2.5
   7f617d559000-7f617d55b000 r--p 00116000 08:01 3861                       /usr/lib/x86_64-linux-gnu/libgcrypt.so.20.2.5
   7f617d55b000-7f617d560000 rw-p 00118000 08:01 3861                       /usr/lib/x86_64-linux-gnu/libgcrypt.so.20.2.5
   7f617d560000-7f617d567000 r--p 00000000 08:01 3900                       /usr/lib/x86_64-linux-gnu/libcryptsetup.so.12.5.0
   7f617d567000-7f617d5b3000 r-xp 00007000 08:01 3900                       /usr/lib/x86_64-linux-gnu/libcryptsetup.so.12.5.0
   7f617d5b3000-7f617d5cc000 r--p 00053000 08:01 3900                       /usr/lib/x86_64-linux-gnu/libcryptsetup.so.12.5.0
   7f617d5cc000-7f617d5ce000 r--p 0006b000 08:01 3900                       /usr/lib/x86_64-linux-gnu/libcryptsetup.so.12.5.0
   7f617d5ce000-7f617d5d0000 rw-p 0006d000 08:01 3900                       /usr/lib/x86_64-linux-gnu/libcryptsetup.so.12.5.0
   7f617d5d0000-7f617d5d2000 r--p 00000000 08:01 3471                       /usr/lib/x86_64-linux-gnu/libcrypt.so.1.1.0
   7f617d5d2000-7f617d5e7000 r-xp 00002000 08:01 3471                       /usr/lib/x86_64-linux-gnu/libcrypt.so.1.1.0
   7f617d5e7000-7f617d601000 r--p 00017000 08:01 3471                       /usr/lib/x86_64-linux-gnu/libcrypt.so.1.1.0
   7f617d601000-7f617d602000 r--p 00030000 08:01 3471                       /usr/lib/x86_64-linux-gnu/libcrypt.so.1.1.0
   7f617d602000-7f617d603000 rw-p 00031000 08:01 3471                       /usr/lib/x86_64-linux-gnu/libcrypt.so.1.1.0
   7f617d60d000-7f617d60f000 r--p 00000000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d60f000-7f617d612000 r-xp 00002000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d612000-7f617d613000 r--p 00005000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d613000-7f617d614000 ---p 00006000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d614000-7f617d615000 r--p 00006000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d615000-7f617d616000 rw-p 00007000 08:01 3482                       /usr/lib/x86_64-linux-gnu/libcap.so.2.32
   7f617d616000-7f617d620000 r--p 00000000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d620000-7f617d656000 r-xp 0000a000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d656000-7f617d666000 r--p 00040000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d666000-7f617d667000 ---p 00050000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d667000-7f617d66c000 r--p 00050000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d66c000-7f617d66d000 rw-p 00055000 08:01 3857                       /usr/lib/x86_64-linux-gnu/libblkid.so.1.1.0
   7f617d66d000-7f617d66f000 r--p 00000000 08:01 3494                       /usr/lib/x86_64-linux-gnu/libacl.so.1.1.2253
   7f617d66f000-7f617d674000 r-xp 00002000 08:01 3494                       /usr/lib/x86_64-linux-gnu/libacl.so.1.1.2253
   7f617d674000-7f617d676000 r--p 00007000 08:01 3494                       /usr/lib/x86_64-linux-gnu/libacl.so.1.1.2253
   7f617d676000-7f617d677000 r--p 00008000 08:01 3494                       /usr/lib/x86_64-linux-gnu/libacl.so.1.1.2253
   7f617d677000-7f617d678000 rw-p 00009000 08:01 3494                       /usr/lib/x86_64-linux-gnu/libacl.so.1.1.2253
   7f617d678000-7f617d67e000 r--p 00000000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d67e000-7f617d697000 r-xp 00006000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d697000-7f617d69e000 r--p 0001f000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d69e000-7f617d69f000 ---p 00026000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d69f000-7f617d6a0000 r--p 00026000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d6a0000-7f617d6a1000 rw-p 00027000 08:01 3840                       /usr/lib/x86_64-linux-gnu/libselinux.so.1
   7f617d6a3000-7f617d6e9000 r--p 00000000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   7f617d6e9000-7f617d85d000 r-xp 00046000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   7f617d85d000-7f617d8ea000 r--p 001ba000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   7f617d8ea000-7f617d8eb000 ---p 00247000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   7f617d8eb000-7f617d8fa000 r--p 00247000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   7f617d8fa000-7f617d8fb000 rw-p 00256000 08:01 3290                       /usr/lib/systemd/libsystemd-shared-245.so
   ```
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
       This has been introduced to solve the security threats caused by the `vsyscall`. In addition, another problem the `vDSO` aim to solve is the limit that the `vsyscall` produce - up to 4 system calls.<br/>
       To sum up, the `vDSO` is dynamically allocated and aim to solve the security concerns such as fixed addresses and it can have more than 4 system calls.<br/>
