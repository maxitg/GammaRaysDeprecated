//
//  GRDistribution.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 07.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRDistribution__
#define __Gamma_Rays__GRDistribution__

#include <iostream>
#include <vector>

using namespace std;

class GRDistribution {
    vector <double> values;
    
    double kolmogorovSmirnovProbability(double distance, int n1, int n2);
public:
    GRDistribution(vector <double> values) : values(values) {};
    
    float kolmogorovSmirnovTest(GRDistribution distribution, double shift = 0., double lengthening = 1.);
};

#endif /* defined(__Gamma_Rays__GRDistribution__) */
