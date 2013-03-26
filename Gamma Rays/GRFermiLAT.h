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
    string fermiDataServerResponce;
    
    static size_t handleFermiDataServerResponce(char *ptr, size_t size, size_t nmemb, GRFermiLAT *me);
    static size_t saveFermiDataServerResponceToFile(char *ptr, size_t size, size_t nmemv, FILE *stream);
    size_t saveFermiDataServerResponce(char *ptr, size_t size, size_t memb);
    
public:
    vector <GRFermiLATPhoton> photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRCelestialSpherePoint location, GRFermiEventClass worstEventClass);
};

#endif /* defined(__Gamma_Rays__GRFermiLAT__) */
