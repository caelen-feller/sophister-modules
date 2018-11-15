#include <iostream> 
#include <iomanip>
#include <cmath>
#include <vector>  

std::vector<double> func(double t, std::vector<double> x, double h)
{
	static const double a = 0.2, b = 0.2, c = 5.7;
	std::vector<double> f(3);

	f[0] = h * ( - x[1] - x[2] );
	f[1] = h * ( x[0] + a * x[1] );
	f[2] = h * ( b + x[2] *(x[0] - c));

	return f;
}

int main()
{
    int d = 3;
	int N = 10000;
	double h, t = 0;
	std::vector<double> x(d);
	x[0] = 0, x[1] = 0, x[2] = 0;
 

    h = 50.0 / double(N);
    x[0] = 0; x[1] = 0; x[2] = 0;

    for (int k = 0; k < N; k++)
    {
        std::vector<double> k1, k2, k3, k4;
        std::vector<double> x1(d), x2(d), x3(d);

        k1 = func(t, x, h);
        for (int i = 0; i < 3; i++) x1[i] = x[i] + 0.5 * k1[i];
        k2 = func(t + 0.5*h, x1, h);
        for (int i = 0; i < 3; i++) x2[i] = x[i] + 0.5 * k2[i];
        k3 = func(t + 0.5*h, x2, h);
        for (int i = 0; i < 3; i++) x3[i] = x[i] + k3[i];
        k4 = func(t + h, x3, h);

        for (int i = 0; i < 3; i++) x[i] += (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]) / 6.0;

        t+=h;
    }

    std::cout << x[0] << " " << x[1] << " " << x[2] << std::setprecision(10) << std::endl;
}
