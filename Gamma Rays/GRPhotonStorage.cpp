//
//  GRPhotonStorage.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include "GRPhotonStorage.h"
#include "GRFermiLAT.h"

vector <GRPhoton> GRPhotonStorage::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location) {
    GRFermiLAT fermiLAT;
    vector <GRFermiLATPhoton> fermiLATPhotons = fermiLAT.photons(startTime, endTime, minEnergy, maxEnergy, location, GRFermiEventClassTransient);
    vector <GRPhoton> result;
    result.reserve(fermiLATPhotons.size());
    for (int i = 0; i < fermiLATPhotons.size(); i++) {
        result.push_back(fermiLATPhotons[i]);
    }
    return result;
}