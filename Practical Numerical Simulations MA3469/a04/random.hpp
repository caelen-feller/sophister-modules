#include <random>
#include <chrono>

namespace pns
{

const double default_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

std::default_random_engine generator(default_seed);

double unif_double(double min, double max)
{
    std::uniform_real_distribution<double> distribution(min,max);
    return distribution(generator);
}

}
