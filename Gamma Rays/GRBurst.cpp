//
//  GRBurst.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include <fstream>
#include <sstream>

#include "GRBurst.h"
#include "GRPhotonStorage.h"
#include "GRDistribution.h"

bool GRBurst::operator<(GRBurst burst) const {
    return time < burst.time;
}

double GRBurst::startTimeLowerBound() {
    return time - durationError;
}

double GRBurst::endTimeUpperBound() {
    return time + duration + durationError;
}

vector <GRPhoton> GRBurst::photons() {
    if (!photonsRetrieved) {
        GRPhotonStorage *storage = NULL;
        storage->getInstance();
        photons_ = storage->photons(startTimeLowerBound(), endTimeUpperBound(), 0, INFINITY, location, locationError);
        photonsRetrieved = true;
    }
    return photons_;
}

int GRBurst::mevCount() {
    GRDistribution mevDistribution = photonDistributionFromStart(0., 1000.);
    return mevDistribution.size();
}

int GRBurst::gevCount() {
    GRDistribution gevDistribution = photonDistributionFromStart(1000., INFINITY);
    return gevDistribution.size();
}

double GRBurst::passTimeOfPhotonsFraction(float fraction) {
    vector <GRPhoton> burstPhotons = photons();
    
    int index = fraction * burstPhotons.size() - 1;
    if (index == -1) return -INFINITY;
    else return burstPhotons[index].time;
}

GRDistribution GRBurst::photonDistributionFromStart(float minEnergy, float maxEnergy) {
    vector <GRPhoton> allPhotons = photons();
    vector <double> times;
    double startTime = passTimeOfPhotonsFraction(START_TIME_FRACTION);
    
    for (int i = 0; i < allPhotons.size(); i++) {
        if (allPhotons[i].energy >= minEnergy && allPhotons[i].energy < maxEnergy) times.push_back(allPhotons[i].time - startTime);
    }
    
    return GRDistribution(times);
}

float GRBurst::gevTransformHypothesisProbability(double shift, double lengthening) {
    GRDistribution mevDistribution = photonDistributionFromStart(0., 1000.);
    GRDistribution gevDistribution = photonDistributionFromStart(1000., INFINITY);
    return mevDistribution.kolmogorovSmirnovTest(gevDistribution, shift, lengthening);
}

double GRBurst::parameterLimit(float probability, GRDistributionParameter parameter, GRDistributionObjective objective, bool *success, bool allowShift, bool allowLengthening) {
    GRDistribution mevDistribution = photonDistributionFromStart(0., 1000.);
    GRDistribution gevDistribution = photonDistributionFromStart(1000., INFINITY);
    return mevDistribution.parameterLimit(gevDistribution, probability, parameter, objective, success, allowShift, allowLengthening);
}

double GRBurst::maxShiftAllowed(float probability, bool *success, bool allowLengthening) {
    return parameterLimit(probability, GRDistributionParameterShift, GRDistributionObjectiveMaximize, success, true, allowLengthening);
}

double GRBurst::minShiftAllowed(float probability, bool *success, bool allowLengthening) {
    return parameterLimit(probability, GRDistributionParameterShift, GRDistributionObjectiveMinimize, success, true, allowLengthening);
}

double GRBurst::maxLengtheningAllowed(float probability, bool *success, bool allowShift) {
    return parameterLimit(probability, GRDistributionParameterLengthening, GRDistributionObjectiveMaximize, success, allowShift, true);
}

double GRBurst::minLengtheningAllowed(float probability, bool *success, bool allowShift) {
    return parameterLimit(probability, GRDistributionParameterLengthening, GRDistributionObjectiveMinimize, success, allowShift, true);
}

string GRBurst::description() {
    ostringstream result;
    
    result << fixed << name << " (" << mevCount() << " " << gevCount() << ")" << " at time " << time << " lasting " << duration << " ± " << durationError << " from " << location.description() << " ± " << locationError;
    
    return result.str();
}

string GRBurst::info() {
    ostringstream result;
    result << fixed;
    
    result << name << endl;
    result << endl;
    
    result << "< GeV photons count: " << mevCount() << endl;
    result << "> GeV photons count: " << gevCount() << endl;
    result << endl;
    
    result << "time:                " << time << endl;
    result << "duration:            " << duration <<  " ± " << durationError << endl;
    result << "5% passed at offset: " << passTimeOfPhotonsFraction(0.05) - time << endl;
    result << endl;
    
    result << "location:       " << location.description() << endl;
    result << "location error: " << locationError << endl;
    result << endl;
    
    if (gevCount() < 10) {
        result << "Too few > GeV photons. Model analysis disabled." << endl << endl;
        return result.str();
    }
    
    bool success;
    int sigmaValues[5] = {1, 2, 3, 4, 5};
    double probabilityValues[5];
    for (int i = 0; i < 5; i++) {
        probabilityValues[i] = 1-erf((double)sigmaValues[i]/sqrt(2.));
    }
    
    result << "Shift Lengthening Model" << endl;
    for (int i = 0; i < 5; i++) {
        result << sigmaValues[i] << " sigma:" << endl;
        double minShift = minShiftAllowed(probabilityValues[i], &success, true);
        double maxShift = maxShiftAllowed(probabilityValues[i], &success, true);
        double avgShift = (maxShift + minShift) / 2.;
        double sprShift = (maxShift - minShift) / 2.;
        double minLengthening = minLengtheningAllowed(probabilityValues[i], &success, true);
        double maxLengthening = maxLengtheningAllowed(probabilityValues[i], &success, true);
        double avgLengthening = (maxLengthening + minLengthening) / 2.;
        double sprLengthening = (maxLengthening - minLengthening) / 2.;
        if (!success) {
            result << "exterminated!" << endl;
        } else {
            result << "shift:       " << avgShift << " ± " << sprShift << endl;
            result << "lengthening: " << avgLengthening << " ± " << sprLengthening << endl;
        }
        result << endl;
    }
    result << endl;
    
    result << "Shift Model" << endl;
    for (int i = 0; i < 5; i++) {
        result << sigmaValues[i] << " sigma:" << endl;
        double minShift = minShiftAllowed(probabilityValues[i], &success, false);
        double maxShift = maxShiftAllowed(probabilityValues[i], &success, false);
        double avgShift = (maxShift + minShift) / 2.;
        double sprShift = (maxShift - minShift) / 2.;
        if (!success) {
            result << "exterminated!" << endl;
        } else {
            result << "shift: " << avgShift << " ± " << sprShift << endl;
        }
        result << endl;
    }
    result << endl;
    
    result << "Lengthening Model" << endl;
    for (int i = 0; i < 5; i++) {
        result << sigmaValues[i] << " sigma:" << endl;
        double minLengthening = minLengtheningAllowed(probabilityValues[i], &success, false);
        double maxLengthening = maxLengtheningAllowed(probabilityValues[i], &success, false);
        double avgLengthening = (maxLengthening + minLengthening) / 2.;
        double sprLengthening = (maxLengthening - minLengthening) / 2.;
        if (!success) {
            result << "exterminated!" << endl;
        } else {
            result << "lengthening: " << avgLengthening << " ± " << sprLengthening << endl;
        }
        result << endl;
    }
    result << endl;
    
    return result.str();
}