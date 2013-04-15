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

struct GRDistributionCDFPoint {
    double value;
    double probability;
};

enum GRDistributionParameter {
    GRDistributionParameterShift = 0,
    GRDistributionParameterLengthening = 1
    };

enum GRDistributionObjective {
    GRDistributionObjectiveMaximize = 0,
    GRDistributionObjectiveMinimize = 1
    };

class GRDistribution {
    vector <double> values;
    
    double kolmogorovSmirnovProbability(double distance, int n1, int n2);
    double kolmogorovSmirnovDistance(double probability, int n1, int n2);
public:
    GRDistribution(vector <double> values) : values(values) {};
    
    int size();
    
    vector <GRDistributionCDFPoint> cdf();
    
    float kolmogorovSmirnovTest(GRDistribution distribution, double shift = 0., double lengthening = 1.);
    double parameterLimit(GRDistribution distribution, float probability, GRDistributionParameter parameter, GRDistributionObjective objective, bool *success, bool allowShift = true, bool allowLengthening = true);
};

#endif /* defined(__Gamma_Rays__GRDistribution__) */
