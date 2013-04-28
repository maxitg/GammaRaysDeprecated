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

#define START_TIME_FRACTION (0.05)
#define START_TIME_OFFSET   (-500)
#define END_TIME_OFFSET     (2000)

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
    
    double startOffset;
    double endOffset;
    
private:
    vector <GRPhoton> photons_;
    bool photonsRetrieved;
    
    double startTimeLowerBound();
    double endTimeUpperBound();
    
public:
    bool operator<(GRBurst burst) const;
    GRDistribution photonDistributionFromStart(float minEnergy, float maxEnergy);    
    GRBurst(string name, double time, GRLocation location, GRBurstType type = GRBurstTypeUndefined) : name(name), time(time), location(location), type(type), photonsRetrieved(0) {};
    
    double passTimeOfPhotonsFraction(float fraction);
    double duration(float fraction);
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
    string info();
};

#endif /* defined(__Gamma_Rays__GRBurst__) */
