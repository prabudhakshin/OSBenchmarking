#define CPUSPEED (2.3*1024*1024*1024)
#define TIMEFORACYCLE (1/CPUSPEED)

double getDurationinSec(double numcycles) {
	return TIMEFORACYCLE*numcycles;
}

double getDurationinMilliSec(double numcycles) {
	return (TIMEFORACYCLE*1000.0)*numcycles;
}

double getDurationinMicroSec(double numcycles) {
	return (TIMEFORACYCLE*1000.0*1000.0)*numcycles;
}
