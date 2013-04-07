//
//  GRDistribution.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 07.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include <fstream>

#include "GRDistribution.h"

double pks(double z) {
    if (z == 0.) return 0.;
    if (z < 1.18) {
        double y = exp(-1.23370055013616983/pow(z,2.));
        return 2.25675833419102515*sqrt(-log(y))*(y + pow(y,9) + pow(y,25) + pow(y,49));
    } else {
        double x = exp(-2.*pow(z,2.));
        return 1. - 2.*(x - pow(x,4) + pow(x,9));
    }
}

double qks(double z) {
	if (z == 0.) return 1.;
	if (z < 1.18) return 1.-pks(z);
	double x = exp(-2.*pow(z, 2.));
	return 2.*(x - pow(x,4) + pow(x,9));
}

double prob(double distance, int n1, int n2) {
    double n = (double)n1*n2 / (n1 + n2);
    return qks((sqrt(n) + 0.12 + 0.11/sqrt(n)) * distance);
}

double GRDistribution::kolmogorovSmirnovProbability(double distance, int n1, int n2) {
    double n = (double)n1*n2/(n1+n2);
    double z = (sqrt(n) + 0.12 + 0.11/sqrt(n)) * distance;
    
    if (z == 0.) return 1.;
    if (z < 1.18) {
        double y = exp(-1.23370055013616983/pow(z,2.));
        return 1. - 2.25675833419102515*sqrt(-log(y))*(y + pow(y,9) + pow(y,25) + pow(y,49));
    } else {
        double x = exp(-2.*pow(z,2.));
        return 2.*(x - pow(x,4) + pow(x,9));
    }
}

float GRDistribution::kolmogorovSmirnovTest(GRDistribution distribution, double shift, double lengthening) {
    for (int i = 0; i < distribution.values.size(); i++) {
        distribution.values[i] += shift;
        distribution.values[i] *= lengthening;
    }
    
    std::ofstream dataMeV("dataMev");
    std::ofstream dataGeV("dataGeV");
    
    for (int i = 0; i < values.size(); i++) {
        dataMeV << values[i] << " " << (double)i/values.size() << endl;
    }
    for (int i = 0; i < distribution.values.size(); i++) {
        dataGeV << distribution.values[i] << " " << (double)i/distribution.values.size() << endl;
    }
    
    dataMeV.close();
    dataGeV.close();
    
    double maxDistance = 0.;
    int index;
    for (int i = 0; i < distribution.values.size(); i++) {
        vector <double>::iterator lowerBound = lower_bound(values.begin(), values.end(), distribution.values[i]);
        double thisIndex = distance(values.begin(), lowerBound);
        double thisHeight = thisIndex / values.size();
        
        double heightLeft = (double)i / distribution.values.size();
        double heightRight = (double)(i+1) / distribution.values.size();
        
        double distanceLeft = thisHeight - heightLeft;
        if (distanceLeft < 0) distanceLeft = -distanceLeft;
        double distanceRight = thisHeight - heightRight;
        if (distanceRight < 0) distanceRight = -distanceRight;
        
        if (distanceLeft > maxDistance) {
            index = i;
            maxDistance = distanceLeft;
        }
        if (distanceRight > maxDistance) {
            index = i;
            maxDistance = distanceRight;
        }
    }
    cout << "index = " << index << endl;
    
    return kolmogorovSmirnovProbability(maxDistance, values.size(), distribution.values.size());
}