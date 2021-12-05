#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>


static inline double getfreqGHz() {
    FILE* fptr = fopen("/proc/cpuinfo", "r");   //"/proc/cpuinfo" is where the cpu MHz located at. Could be received within "cat /proc/cpuinfo | grep cpu"
    if (fptr == NULL) {
        printf("ERROR: File PATH /proc/cpuinfo does not exists");
        return EXIT_FAILURE;
    }

    char temp1, temp2;
    double mhz = 0;                             //A variable for storing the frequency of CPU in MHz from the cpuinfo file
    size_t n = 0;
    char* line = NULL;
    while (getline(&line, &n, fptr) > 0) {      //A loop for searching the MHz in the /proc/cpuinfo file and insert it into the double mhz variable
        if (strstr(line, "MHz")) {
            sscanf(line, "%s %s : %lf", &temp1, &temp2, &mhz);
            break;
        }
    }
//  printf("CPU in MHz: %lf \n", mhz); 		//Added only for print-debugging
//  printf("CPU in GHz: %lf \n", (mhz/1000));	//Added only for print-debugging
    free(line);
    fclose(fptr);
    return (mhz / 1000);
}
    
unsigned long long gethosttime(unsigned long long cycles) {
    double ghz =  getfreqGHz();
//  printf("The equivalent in nanoseconds: %lf \n", cycles / ghz); //Added only for print-debugging
    return (cycles / ghz);
}

static inline unsigned long long getcycles(void) {
    unsigned long low, high;
    asm volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((low) | (high) << 32);
}

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

int main(int argc, char** argv) {
    measure_get_cycles();
    measure_gettimeofday();
    calculate_inner_loop();
    return 0;
}
