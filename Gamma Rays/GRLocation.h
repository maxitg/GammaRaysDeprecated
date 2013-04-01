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
#include <vector>

using namespace std;

enum GRCoordinateSystem {
    GRCoordinateSystemJ2000 = 0,
    GRCoordinateSystemGalactic = 1
    };

class GRLocation {
public:
    float ra;
    float dec;
    
public:
    GRLocation(GRCoordinateSystem system, float ra, float dec);
    GRLocation();
    string description();
};

#endif /* defined(__Gamma_Rays__GRCelestialSpherePoint__) */
