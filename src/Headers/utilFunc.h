#include <stdio.h>

void printResult(int samples, double mean, double stddev, char* header) {
	printf("\n");
	printf("---------------------------------------------\n");
	printf("              %s              \n", header);
	printf("---------------------------------------------\n");
	printf("%11s %d\n", "NumSamples:", samples);
	printf("%11s %f\n", "Mean:", mean);
	printf("%11s %f\n", "StdDev:", stddev);
	printf("---------------------------------------------\n");
}
