#include "Utilities.h"

static inline double recall(double tp, double fn) 
{
    return tp /( tp + fn);
}

static inline double precision(double tp, double fp) 
{
    return tp/(tp + fp);
}

static inline double accuracy( double tp, double tn, double total) 
{
    return (tp + tn) / total;
}