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
    double duration;
    double durationError;
    GRLocation location;
    float locationError;
    GRBurstType type;
    
private:
    vector <GRPhoton> photons_;
    bool photonsRetrieved;
    
    double startTimeLowerBound();
    double endTimeUpperBound();
    
public:
    bool operator<(GRBurst burst) const;
    GRDistribution photonDistributionFromStart(float minEnergy, float maxEnergy);    
    GRBurst(string name, double time, double duration, double durationError, GRLocation location, float locationError, GRBurstType type = GRBurstTypeUndefined) : name(name), time(time), duration(duration), durationError(durationError), location(location), locationError(locationError), type(type), photonsRetrieved(0) {};
    
    double passTimeOfPhotonsFraction(float fraction);
    int mevCount();
    int gevCount();
    
    float gevTransformHypothesisProbability(double shift, double lengthening);
    
    double parameterLimit(float probability, GRDistributionParameter parameter, GRDistributionObjective objective, bool *success, bool allowShift = true, bool allowLengthening = true);
    
    double maxShiftAllowed(float probability, bool *success, bool allowLengthening = true);
    double minShiftAllowed(float probability, bool *success, bool allowLengthening = true);
    double maxLengtheningAllowed(float probability, bool *success, bool allowShift = true);
    double minLengtheningAllowed(float probability, bool *success, bool allowShift = true);
    
    vector <GRPhoton> photons();
    
    string description();
};

#endif /* defined(__Gamma_Rays__GRBurst__) */
