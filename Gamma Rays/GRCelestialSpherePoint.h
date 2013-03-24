//
//  GRCelestialSpherePoint.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRCelestialSpherePoint__
#define __Gamma_Rays__GRCelestialSpherePoint__

#include <iostream>
#include <string>

using namespace std;

class GRCelestialSpherePoint {
public:
    float ra;
    float dec;
    
public:
    GRCelestialSpherePoint(float ra, float dec) : ra(ra), dec(dec) {};
    
    string description();
};

#endif /* defined(__Gamma_Rays__GRCelestialSpherePoint__) */
