//
//  GRBurst.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include "GRBurst.h"
#include "GRPhotonStorage.h"
#include "GRDistribution.h"

double GRBurst::startTimeLowerBound() {
    return time + GRBURST_START_TIME_LOWER_BOUND_OFFSET;
}

double GRBurst::endTimeUpperBound() {
    return time + GRBURST_END_TIME_UPPER_BOUND_OFFSET;
}

vector <GRPhoton> GRBurst::photons() {
    if (!photonsRetrieved) {
        GRPhotonStorage *storage = NULL;
        storage->getInstance();
        photons_ = storage->photons(startTimeLowerBound(), endTimeUpperBound(), 0, INFINITY, location);
        photonsRetrieved = true;
    }
    return photons_;
}

double GRBurst::passTimeOfPhotonsFraction(float fraction) {
    vector <GRPhoton> burstPhotons = photons();
    
    int index = fraction * burstPhotons.size() - 1;
    if (index == -1) return -INFINITY;
    else return burstPhotons[index].time;
}

float GRBurst::gevTransformHypothesisProbability(double shift, double lengthening) {
    vector <GRPhoton> allPhotons = photons();
    vector <double> mevTimes;
    vector <double> gevTimes;
    
    double startTime = passTimeOfPhotonsFraction(START_TIME_FRACTION);
    
    for (int i = 0; i < allPhotons.size(); i++) {
        if (allPhotons[i].energy > 1000) gevTimes.push_back(allPhotons[i].time - startTime);
        else mevTimes.push_back(allPhotons[i].time - startTime);
    }
    
    GRDistribution mevDistribution(mevTimes);
    GRDistribution gevDistribution(gevTimes);
    return mevDistribution.kolmogorovSmirnovTest(gevDistribution, shift, lengthening);
}