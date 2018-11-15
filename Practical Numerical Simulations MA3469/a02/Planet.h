#include <cmath>
#include <iostream>
#include <iomanip>

class Planet
{
	private:
		double m;
	public:
		double x[2];
		double v[2];

		Planet(double mass, double x0, double x1, double v0, double v1) : m(mass)
		{
			x[0] = x0; x[1] = x1;
			v[0] = v0; v[1] = v1;
		}

		~Planet()
		{
		}

		double mass() const { return m; } 
		
        void print();
};
