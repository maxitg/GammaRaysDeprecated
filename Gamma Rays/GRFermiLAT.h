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
#include "GRPsf.h"

class GRFermiLAT {
    string fermiDataServerResponce;
    
    static size_t handleFermiDataServerResponce(char *ptr, size_t size, size_t nmemb, GRFermiLAT *me);
    static size_t saveFermiDataServerResponceToFile(char *ptr, size_t size, size_t nmemv, FILE *stream);
    size_t saveFermiDataServerResponce(char *ptr, size_t size, size_t memb);
    char digitHex(int digit);
    string hash(double startTime, double endTime, GRLocation location);
    
    string gtselect(string queryHash);
    string gtmktime(string queryHash);
    string gtltcube(string queryHash);
    string gtpsf(string queryHash, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    
    string instrumentResponceFunctionName(GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    
    bool fileExists(string queryHash, string fileName);
    
    void processPhotons(string queryHash);
    string downloadPhotons(double startTime, double endTime, GRLocation location);
    
    GRFermiEventClass eventClassFromPsfInt(int psfInt);
    GRFermiConversionType conversinoTypeFromPsfInt(int psfInt);
    vector <GRFermiLATPhoton> psfUnfilteredPhotons(double startTime, double endTime, GRLocation location);
    
public:
    GRPsf psf(double startTime, double endTime, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    vector <GRPhoton> photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, GRFermiEventClass worstEventClass, float confidence);
};

#endif /* defined(__Gamma_Rays__GRFermiLAT__) */
