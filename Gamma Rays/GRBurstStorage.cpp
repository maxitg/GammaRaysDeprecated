//
//  GRBurstStorage.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 14.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include "GRBurstStorage.h"
#include "GRFermiGBM.h"

vector <GRBurst> GRBurstStorage::bursts(double startTime, double endTime) {
    GRFermiGBM fermiGRB;
    vector <GRBurst> bursts = fermiGRB.bursts(startTime, endTime);
    sort(bursts.begin(), bursts.end());
    return bursts;
}