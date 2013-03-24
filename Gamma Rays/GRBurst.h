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
#include <vector>
#include <string>

#include "GRPhoton.h"
#include "GRCelestialSpherePoint.h"

using namespace std;

#define GRBURST_START_TIME_LOWER_BOUND_OFFSET (-500)
#define GRBURST_END_TIME_UPPER_BOUND_OFFSET   (2000)

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
    
    double startTimeLowerBound();
    double endTimeUpperBound();
    
public:
    GRBurst(string name, double time, float ra, float dec, GRBurstType type = GRBurstTypeUndefined) : name(name), time(time), location(GRCelestialSpherePoint(ra, dec)), type(type) {};
    
    vector <GRPhoton> photons();
};

#endif /* defined(__Gamma_Rays__GRBurst__) */
