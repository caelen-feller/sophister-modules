#include <iostream> 
#include <iomanip>
#include <cmath>

double euler(double (*f)(double, double), const double &t0, const double &x0, const double &t1, const int &N)
{   
	double xk = x0, tk = t0;
  double h = (t1-t0) / double(N);

	for (int i = 1; i <= N; i++)
	{
		xk = xk + h * f(xk, tk);

		tk += h;
	}

	return xk;
}

double rk4(double (*f)(double, double), const double &t0, const double &x0, const double &t1, const int &N)
{   
	double xk = x0, tk = t0;
  double h = (t1-t0) / double(N);
	double k1, k2, k3, k4;

	for (int i = 1; i <= N; i++)
	{
		k1 = h * f(xk, tk);
		k2 = h * f(xk + k1 / 2.0, tk + h / 2.0);
		k3 = h * f(xk + k2 / 2.0, tk + h / 2.0);
		k4 = h * f(xk + k3, tk + h);

		xk = xk + (k1 + 2.0*k2 + 2.0*k3 + k4) / 6.0;

		tk = tk + h;
	}

	return xk;
}


double f(double x, double t)
{
  return t*(pow(x,2) - 1.0);
}

double solution(double t)
{
	return (1.0 - exp(pow(t,2.0)))/(1.0 + exp(pow(t,2.0)));
}

int main()
{
  double t0=0,t1=2;
  double x0=0;
  std::cout << std::setprecision(5);

  // for(int i = 20; i <= 2000; i++)
  // {
  //   std::cout << (t1-t0) / double(i) << "   " << fabs(solution(t1) - euler(f, t0, x0, t1, i)) << std::endl;
  // }

 	for(int i = 20; i <= 2000; i++)
  {
    std::cout << (t1-t0) / double(i) << "   " << fabs(solution(t1) - rk4(f, t0, x0, t1, i))  << std::endl;
  }
}