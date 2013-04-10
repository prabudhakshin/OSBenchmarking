#define __MATHHEADER__
#include <math.h>

double getMeanStddev(double* inputArray, int samplecount, double* stddev) {

	FILE* fd = fopen("./dump.txt", "w");
	int index = 0, omittedSamples = 0;
	double sum = 0, mean = 0, t = 0;
	double meansqrdsum = 0;

	for (index = 0; index < samplecount; index++) {
		t = sum;
		fprintf(fd, "%f\n", inputArray[index]);

		if (inputArray[index]<0) {
			omittedSamples++;
			continue;
		}

		sum += inputArray[index];

		if (sum < t) {
			printf("Overflow! Exiting!!");
			exit(2);
		}
	}

	//printf("OmitSam = %d\tSum = %f\n", omittedSamples, sum);
	fclose(fd);

	mean = sum/(samplecount - omittedSamples);

	omittedSamples = 0;
	for (index = 0; index < samplecount; index++) {
		t = meansqrdsum;

		if (inputArray[index]<0) {
			omittedSamples++;
			continue;
		}

		meansqrdsum += pow ((inputArray[index] - mean), 2);

		if (meansqrdsum < t) {
			printf("Overflow while calculating meansquared sum! Exiting!!");
			exit(2);
		}
	}

	*stddev = sqrt((double)meansqrdsum/(samplecount - omittedSamples));

	return mean;
}
