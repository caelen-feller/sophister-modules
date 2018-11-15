#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>


// The Differential Equation presented in the question
std::vector<double> func(double t, std::vector<double> x, double h)
{
    static const double a=0.09, b=0.1;
    std::vector<double> r(2);

    r[0] = h*(x[1]);
    r[1] = -h*( a*x[1] + pow(x[0],3.0)*exp( -b*t*pow(x[0], 2.0) ) );

    return r;
}

// RK4 method, but modified to work on vectors! 
std::vector<double> rk4(std::vector<double> (*update)(double, std::vector<double>, double), std::vector<double> x0, double t0, double t1, double N)
{
		int d = x0.size();
		std::vector<double> x(x0);
		double h = (t1-t0)/double(N), t=t0;

		std::vector<double> k1, k2, k3, k4;
		std::vector<double> x1(d), x2(d), x3(d);

		for(int k = 0; k < N; k++)
		{
				k1 = update(t, x, h);
				for (int i = 0; i < d; i++) x1[i] = x[i] + 0.5 * k1[i];
				k2 = update(t + 0.5*h, x1, h);
				for (int i = 0; i < d; i++) x2[i] = x[i] + 0.5 * k2[i];
				k3 = update(t + 0.5*h, x2, h);
				for (int i = 0; i < d; i++) x3[i] = x[i] + k3[i];
				k4 = update(t + h, x3, h);
				
				for (int i = 0; i < d; i++) x[i] += (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]) / 6.0;
				
				t+=h;
		}
		
		return x;
}

// Root finding Bisector
double bisect(double (*b)(double), double c_lo, double c_hi)
{
	double b_lo = b(c_lo);
	double b_hi = b(c_hi);
	double c;

	if (b_hi*b_lo > 0.0) return c_lo;

	do
	{
		c = 0.5 * (c_hi+c_lo);
		double bc = b(c); 
		if (bc*b_lo > 0.0) 
		{
			c_lo = c; b_lo = bc;
		}
		else 
		{
			c_hi = c; b_hi = bc;
		}
	}
	while (c_hi-c_lo > 10e-10); 
	return 0.5*(c_hi+c_lo);
}

// Just gathering some metadata
struct meta {
	double x0=0, x1=-1;
	double t0=0,t1=20;
	int N = 1000;
} meta;

// The error given a guessed slope for a shot solution
double b(double c)
{
	std::vector<double> x = {meta.x0, c};
	return rk4(func, x, meta.t0, meta.t1, meta.N)[0] - meta.x1;
}

int main()
{
	std::cout << std::setprecision(7);

	// Bounds and resolution for root searching determined by visual inspection
	double c_lo = -30, c_hi=30, c_res =0.5;
	
	// Finds the zeros, to 6 sf!
	std::vector<double> zeros;
	for(double c = c_lo; c<30; c += c_res)
	{
		double zero = bisect(b, c, c + c_res);
   		if(zero != c) 
		{
			zeros.push_back(zero);
			std::cout << zero  << std::endl ;
		}
	}   

	// Code for plotting the solutions,
	// (not part of the software specification, but thought I'd include it)
	// std::cout << std::endl;
	// for(double t = meta.t0; t < meta.t1; t += 0.05)
	// {
	// 	std::cout << t << " ";
		
	// 	for(int i = 0; i<zeros.size(); i++)
	// 	{
	// 		std::vector<double> x = {meta.x0, zeros[i]};
	// 		std::cout << rk4(func, x, meta.t0, t, meta.N)[0] << " "; 
	// 	}

	// 	std::cout << std::setprecision(10) << std::endl;
	// }

}