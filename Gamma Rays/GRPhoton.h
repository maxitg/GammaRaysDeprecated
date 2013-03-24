//
//  GRPhoton.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRPhoton__
#define __Gamma_Rays__GRPhoton__

#include <iostream>
#include <string>

#include "GRCelestialSpherePoint.h"

using namespace std;

class GRPhoton {
protected:
    GRCelestialSpherePoint location;
    float energy;
    double time;
    
    string energyDescription();
    
public:
    GRPhoton(double time, float ra, float dec, float energy) : time(time), location(GRCelestialSpherePoint(ra, dec)), energy(energy) {};
    
    string description();
};

#endif /* defined(__Gamma_Rays__GRPhoton__) */
