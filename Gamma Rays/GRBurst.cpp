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

double GRBurst::startTimeLowerBound() {
    return time + GRBURST_START_TIME_LOWER_BOUND_OFFSET;
}

double GRBurst::endTimeUpperBound() {
    return time + GRBURST_END_TIME_UPPER_BOUND_OFFSET;
}

vector <GRPhoton> GRBurst::photons() {
    GRPhotonStorage *storage = NULL;
    storage->getInstance();
    return storage->photons(startTimeLowerBound(), endTimeUpperBound(), 0, INFINITY, location);
}