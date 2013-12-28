#ifndef __SPLINE_H
#define __SPLINE_H

#define SPLINEN 11

int spline (int n, int end1, int end2,
            double slope1, double slope2,
            double x[SPLINEN], double y[SPLINEN],
            double b[SPLINEN], double c[SPLINEN], double d[SPLINEN],
            int *iflag);
						
double seval (int n, double u,
              double x[SPLINEN], double y[SPLINEN],
              double b[SPLINEN], double c[SPLINEN], double d[SPLINEN],
              int *last);
							
double deriv (int n, double u,
              double x[SPLINEN],
              double b[SPLINEN], double c[SPLINEN], double d[SPLINEN],
              int *last);
							
double sinteg (int n, double u,
              double x[SPLINEN], double y[SPLINEN],
              double b[SPLINEN], double c[SPLINEN], double d[SPLINEN],
              int *last);							

#endif
