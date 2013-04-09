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
#include "GRLocation.h"
#include "GRDistribution.h"

using namespace std;

#define GRBURST_START_TIME_LOWER_BOUND_OFFSET (-20)
#define GRBURST_END_TIME_UPPER_BOUND_OFFSET   (200)
#define START_TIME_FRACTION                   (0.05)

enum GRBurstType {
    GRBurstTypeUndefined = 0,
    GRBurstTypeShort = 1,
    GRBurstTypeLong = 2
    };

class GRBurst {
public:
    string name;
    double time;
    GRLocation location;
    GRBurstType type;
    
private:
    vector <GRPhoton> photons_;
    bool photonsRetrieved;
    
    double startTimeLowerBound();
    double endTimeUpperBound();
    
    GRDistribution photonDistributionFromStart(float minEnergy, float maxEnergy);

public:
    GRBurst(string name, double time, GRCoordinateSystem system, float ra, float dec, GRBurstType type = GRBurstTypeUndefined) : name(name), time(time), location(GRLocation(system, ra, dec)), type(type), photonsRetrieved(0) {};
    
    double passTimeOfPhotonsFraction(float fraction);
    int gevCount();
    
    float gevTransformHypothesisProbability(double shift, double lengthening);
    
    double parameterLimit(float probability, GRDistributionParameter parameter, GRDistributionObjective objective, bool *success, bool allowShift = true, bool allowLengthening = true);
    
    double maxShiftAllowed(float probability, bool *success, bool allowLengthening = true);
    double minShiftAllowed(float probability, bool *success, bool allowLengthening = true);
    double maxLengtheningAllowed(float probability, bool *success, bool allowShift = true);
    double minLengtheningAllowed(float probability, bool *success, bool allowShift = true);
    
    vector <GRPhoton> photons();
};

#endif /* defined(__Gamma_Rays__GRBurst__) */
