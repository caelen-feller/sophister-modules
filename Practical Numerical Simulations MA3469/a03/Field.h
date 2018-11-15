#include <vector>
#include <iostream>
#include <iomanip>

template <class T>
class Field
{
	private:
		int nx;
		int ny;
		std::vector<T> data; 
		
	public:
		Field(int nx, int ny) : nx(nx), ny(ny), data (nx * ny) { }
		
		Field(int nx, int ny, T val) : nx(nx), ny(ny), data (nx * ny ) 
		{
			for (int i=0;i < nx * ny; i++) data[i] = val;
		}

		T& operator() (int x, int y) 
		{
			return data[x + nx*y];
		}

		T operator() (int x, int y)  const
		{
			return data[x + nx*y];
		}

		int width() const { return nx; }
		int height() const { return ny; }
};


template <class T> std::ostream& operator<< (std::ostream& os, const Field<T>& f)
{
    for(int y = f.height() - 1;  y >= 0; y--)
    {
        for(int x = 0;  x < f.width(); x++)
        {
            os << (float)x/(float)(f.width()-1) << " " << (float)y/(float)(f.height()-1)<< " " << f(x,y) << std::endl;
        }    
		os << std::endl;
    }
	return os;
}