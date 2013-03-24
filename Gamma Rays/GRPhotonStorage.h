//
//  GRPhotonStorage.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRPhotonStorage__
#define __Gamma_Rays__GRPhotonStorage__

#include <iostream>
#include <vector>

#include "GRPhoton.h"

using namespace std;

class GRPhotonStorage {
public:
    static GRPhotonStorage& getInstance() {
        static GRPhotonStorage instance;
        return instance;
    }
    
private:
    GRPhotonStorage() {};
    GRPhotonStorage(GRPhotonStorage const&);
    void operator=(GRPhotonStorage const&);
    
public:
    vector <GRPhoton> photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRCelestialSpherePoint direction);
};

#endif /* defined(__Gamma_Rays__GRPhotonStorage__) */
