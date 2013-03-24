//
//  GRBurst.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRBurst__
#define __Gamma_Rays__GRBurst__

#include <stdio.h>

#include <iostream>
#include <string>

#include "GRCelestialSpherePoint.h"

using namespace std;

enum GRBurstType {
    GRBurstTypeUndefined = 0,
    GRBurstTypeShort = 1,
    GRBurstTypeLong = 2
    };

class GRBurst {
    string name;
    double time;
    GRCelestialSpherePoint location;
    GRBurstType type;
    
public:
    GRBurst(string name, double time, float ra, float dec, GRBurstType type = GRBurstTypeUndefined) : name(name), time(time), location(GRCelestialSpherePoint(ra, dec)), type(type) {};
};

#endif /* defined(__Gamma_Rays__GRBurst__) */
