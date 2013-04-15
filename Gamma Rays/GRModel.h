//
//  GRModel.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 12.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRModel__
#define __Gamma_Rays__GRModel__

#include <iostream>

class GRModel {
    double jetLength;
    int jetLengthIndex;
    double thetaCoefficient;
    int thetaIndex;
    double earthAngle;
    double jetRelativisticFactor;
    
    double jetSpeed;
    
public:
    double density(double r, double theta, double energy);
    double flux(double t, double energy);
};

#endif /* defined(__Gamma_Rays__GRModel__) */
