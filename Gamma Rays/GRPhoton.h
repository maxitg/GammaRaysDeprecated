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

#include "GRLocation.h"

using namespace std;

class GRPhoton {
public:
    GRLocation location;
    float energy;
    double time;

protected:
    string energyDescription();
    
public:
    GRPhoton(double time, GRLocation location, float energy) : time(time), location(location), energy(energy) {};
    
    bool operator<(GRPhoton right) const;
    
    string description();
};

#endif /* defined(__Gamma_Rays__GRPhoton__) */
