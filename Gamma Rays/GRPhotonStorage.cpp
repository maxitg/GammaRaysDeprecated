//
//  GRPhotonStorage.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <fstream>

#include "GRPhotonStorage.h"
#include "GRFermiLAT.h"

vector <GRPhoton> GRPhotonStorage::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location) {
    GRFermiLAT fermiLAT;
    vector <GRPhoton> fermiLATPhotons = fermiLAT.photons(startTime, endTime, minEnergy, maxEnergy, location, GRFermiEventClassTransient, 0.95);
    sort(fermiLATPhotons.begin(), fermiLATPhotons.end());
        
    return fermiLATPhotons;
}