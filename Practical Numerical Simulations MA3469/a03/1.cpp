#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>

#include "Field.h"

int fixed(int x, int y, int s, int n)
{
    // fixed
    if( x == 0 || y == 0 || x == n-1 || y == n-1 )  return 1;
    
    // A
    if( y == 2*s && x > 1*s && x < 7*s) return 2;

    // B
    if( y == 8*s && x > 1*s && x < 9*s) return 3;
    if( x == 8*s && y > 1*s && y < 9*s) return 3;
    
    return 0;
}


int main()
{
    int s = 10; 
    int n = (9*s)+2;
    
    Field<double> phi(n,n,0);

    // Set Boundaries
    for(int y = 0;  y < n; y++)
    {
        for(int x = 0;  x < n; x++)
        {
            int region = fixed(x,y,s,n);
            if( region  == 2) phi(x,y) = 1;

            if( region == 3) phi(x,y) = -1;
        }
    }

    // SOR Iteration
    double omega = 1.99;
    int N = 0;
    double diff = 1.0; 
    while (diff > pow(10,-4)) 
    {
        diff = 0.0;
        for(int y = 0;  y < n; y++)
        {
            for(int x = 0;  x < n; x++)
            {
                if(!fixed(x,y,s,n)) 
                { 
                    double phidiff = ( phi(x-1,y) + phi(x+1, y) + phi(x, y-1) + phi(x,y+1) ) / 4.0 - phi(x,y);
                    diff += fabs(phidiff);

                    // Over-Relaxation
                    phi(x,y) += omega*phidiff;
                }
            }
        }
        N++;
    }
    std::cerr << N << " " << diff << std::endl;

    // Approximate derivative
    std::cerr << (phi(2*s,5*s) - phi(2*s+1,5*s))*double(n) << std::endl;
    
    // Output Graph
    std::cout << phi; 
}
