# Homework 2

**Advanced Operating Systems, Term 1, 2022** <br/>
**Updated**: Thursday 12.11.2021 at 09:11 IDT <br/>
**Due**: Thursday, 25.11.2021 at 11:59pm IDT

## Instructions

Group collaboration (up to 2 people) is permitted on the kernel programming
problems. All other problems in this assignment are to be done individually.
Submission of all homework is recommended to be made via Git.

## Individual Problems:

### (1) Timing methodologies

The x86 has a TSC hardware register that counts processor cycles. It can be
read using the _rdtsc_ instruction to get very accurate timing measurements.
Several macros can be used to read this register (see include file _asm/msr.h_).
The function _getcycles()_ below returns the number of cpu cycles used since
the system was last booted.

        static inline unsigned long long getcycles(void)
        {
            unsigned long low, high;
            asm volatile ("rdtsc" : "=a" (low), "=d" (high));
            return ((low) | (high) << 32);
        }

(a) Write a function in C called _gethosttime()_ that takes a long long cycles
value as an argument and returns the equivalent long long in nanoseconds. To be
portable, it should determine the CPU speed of the machine where it is called.
In Linux, you can use the data in _/proc/cpuinfo_ for this.

(b) Using _gethosttime()_, measure how long it takes to execute _getcycles()_.
In addition, time the _gettimeofday()_ system call. You should be mindful to do
the measurement while minimizing the overhead of doing the measurement.

(c) Use both _gethosttime()_ and the _gettimeofday()_ system call to time the
inner for-loop of the following bit of code:

        for (i=0; i < 1000; i++) {
            for (j=0; j < 100; j++) {  /* inner loop starts here */
                k = i + j;  
            }                          /* inner loop ends here */
        }

The answer should indicate the mean and standard deviation of loop iterations,
and explain large variations in the results, if any. Based on the measurements,
indicate which timing method is more accurate and explain why. 

### (2) Understanding of process memory maps

(a) Explain the page table mappings of each region of _cat /proc/self/maps_ by
stating precisely what each region is used for (e.g. [stack] is the stack of
the process).

(b) Explore the page table mappings of _systemd-journald_ using the command
_cat /proc/$(pidof systemd-journald)/maps_. Explain the regions types that you
have not seen in (a).

(c) Do _cat /proc/self/maps_ multiple times on your system (laptop or VM). What
do you notice regarding the page table mapping? Why?

(d) Notice the _[vdso]_ mapping that is common to all processe. Read the man
page (_man vdso_), and explain its purpose and the problems it aims to solve.

## Group kernel programming:

The kernel programming will be done using a Linux VM. You are encouraged to
reuse the VM that you had created for Homework 1.

Memory can be a limited resource and it is important for the OS to manage it
efficiently. It is helpful to understand how and what the OS is doing in this
context depending on how programs allocate and use memory. In this assignment
you will write new logic to measure and visualize how memory is managed and
used, along with user programs that create different memory access patterns.

### (1) Page Table Statistics

The special file _/proc/[pid]/maps_ (_[pid]_ can be _self_) gives information
on the memory regions used by a process. (The output is dynamically generated).
For details, read the corresponding section in _man 5 proc_.

Write a system call to report some page table statistics for regions of the
process's memory address space within an address range specified by the caller.
The system call should take three arguments, and copy the generated textual
report to a buffer provider by the caller.

The prototype for the new system call will be:

        int mapspages(unsigned long start, unsigned long end, char __user *buf, size_t size);

Parameters description

- _start_ is the (userspace) starting virtual address of memory range for which
  to generate the report (inclusive).

- _end_ is the (userspace) ending virtual address of memory range for which to
  generate the report (exclusive).

- _buf_ points to a buffer to store the textual report (see below for details
  on the report format).

- _size_ indicates the size of the buffer in bytes. The system call copies at
  most as many entries that fit the buffer (in full) without breaking any. 

- Return value: returns the number of bytes copied. If this value is equal to
  the size of the buffer, then the caller should retry with a larger buffer.

The report should be similar to that of _/proc/maps_, with additional details
on the pages associated with each region. More specifically, the report should
contain one line for each memory region, in the following format:

        5602f6f93000-5602f6f95000 r--p 00000000 fd:02 1146440 <pages-details>

Where the first 5 fields are like those of _/proc/maps_, and the last field
(\<pages-details\>) shows one character per page table entry in that region,
as follows (the length of the string is the number of pages in that region):

        '.'   if the page is not present
        '1-9' to represent the number of references existing for the respective physical page
        'X'   if the number of references for the respective physical page exceeds 9 

For example, the string _'....1'_ represents a memory region with four pages
that have not yet been mapped followed by one page that is referenced once.
You should ignore regions allocated with MAP\_HUGETLB (see _man mmap_).

The code should handle errors that could occur. It should detect and handle at
least the following:

- -EINVAL: if _start > end_.
- -EFAULT: if _buf_ is outside the accessible address space.

You may implement the logic either entirely in the kernel, or using a kernel
module like in Assignment 1 (for faster development-test cycles).
    
### (2) Test the new system call

Write a C program which calls _mapspages()_ with suitable arguments to generate
the following test cases:

- Test 1: prints a region with ".........."
- Test 2: prints a region with "1111111111"
- Test 3: prints a region with ".1.1.1.1.1"
- Test 4: prints a region with "22222....."
- Test 5: prints a region with "1111..2222"
- Test 6: prints a region with exactly 2000 '.' and nothing else.
- Test 7: prints the stack region such that the sum of the printed digits is
  maximal ('.' and 'X' count as 0). You may have to use something other than
  _fork()_.
- Test 8: takes a random string (of only _[.0-9X]_ characters) as an argument
  and prints a region with exactly that string.
- Test 9: triggers the Linux OOM (Out-Of-Memory) killer.

For Test 8, you can find the stack top address by looking at _/proc/self/stat_
(see the respective section in _man 5 proc_); and the bottom address using some
local variable.

The test program will also have a function _print\_maps(start, end)_ which
dumps the current process mapping in the specific region. You will have to use
_mmap()_.  You may not use _MAP\_SHARED_.

