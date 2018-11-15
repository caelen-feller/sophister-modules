#include "Planet.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>

// Computes distance between two planets
static inline double dist(Planet a, Planet b)
{
    double sum = 0;
    for(int i =0; i<2; i++)
        sum += pow(a.x[i] - b.x[i], 2);
    return sqrt(sum);
}

// Update positions given a velocity
void update_positions(double h, std::vector<Planet>& s)
{
	int n=s.size();
	for (int i=0;i<n;i++)
	{
		for (int a=0;a<2;a++) s[i].x[a] += h * s[i].v[a]; 
	}
}

// Update velocities given weights
void update_velocities(double h, std::vector<Planet>& s)
{
	int n=s.size();
    double f[2], mag;    
	for (int i=0;i<n;i++)
	{
        f[0]=0;f[1]=0;

        for(int j =0; j<n; j++)
        {
            if(i==j) continue;
            
            // Calculates F_ij = (mi*mj / rij^2)*(xi-xj)/rij
            mag = s[i].mass()*s[j].mass()/pow(dist(s[i], s[j]),3.0);
            for (int a=0;a<2;a++) 
                f[a] += mag*(s[j].x[a] - s[i].x[a]);
        }

        for (int a=0;a<2;a++) 
            s[i].v[a] += h * f[a] / s[i].mass();
	}
}

// Does some formatting and output
void print_planets(double t, std::vector<Planet>& s)
{
    for(int j = 0; j<s.size(); j++)
    {
        std::cout << j << " ";
        s[j].print();
    }
    std::cout << std::endl;
}

void Planet::print()
{
    std::cout << x[0] << " " << x[1] << std::endl;
}

int main()
{
	std::vector<Planet> s; 
	s.push_back(Planet(2.2, -0.50, 0.10, -0.84, 0.65));
	s.push_back(Planet(0.8, -0.60, -0.20, 1.86, 0.70));
	s.push_back(Planet(0.9, 0.50, 0.10, -0.44, -1.50));
	s.push_back(Planet(0.4, 0.50, 0.40, 1.15, -1.60));

    int N = 100000;
    double t0 = 0, t1 = 4, t=t0;
    double h = (t1-t0)/double(N);
    

    print_planets(t, s);
    for(int i=0; i<N; i++)
    {
        update_positions(h/2.0, s);
        update_velocities(h, s);
        update_positions(h/2.0, s);
        t+=h;
    }
    print_planets(t, s);
}
