//
//  GRFermiLAT.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRFermiLAT__
#define __Gamma_Rays__GRFermiLAT__

#include <iostream>
#include <vector>

#include "GRFermiLATPhoton.h"

class GRFermiLAT {
    
    
public:
    vector <GRFermiLATPhoton> photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRCelestialSpherePoint location, GRFermiEventClass worstEventClass);
};

#endif /* defined(__Gamma_Rays__GRFermiLAT__) */
