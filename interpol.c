#include "interpol.h"

double interpol(double z, double x[POLYN_DEG], double y[POLYN_DEG]){
	double wnz = 0, om = 1, w;
	int i, j;
	for(i = 0; i <= POLYN_DEG; i++){
		if(z == x[i])return y[i];
		om = om * (z - x[i]);
		w = 1.0;
		for(j = 0; j<= POLYN_DEG; j++)
			if( i!=j ) w = w * (x[i] - x[j]);
		wnz = wnz + y[i] / (w * (z - x[i]));;
	}
	return wnz = wnz * om;
}
