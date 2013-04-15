//
//  GRModel.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 12.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include "GRModel.h"

double GRModel::density(double r, double theta, double energy) {
    return 1./(1. + pow(r/jetLength, jetLengthIndex)) * exp(-pow(theta/(thetaCoefficient*pow(energy, thetaIndex)), 2.));
}

double GRModel::flux(double t, double energy) {
    
}