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

enum GRFermiLATException {
    GRFermiLATExceptionMkdir,
    GRFermiLATExceptionFileOpen,
    GRFermiLATExceptionSymlink,
    
    GRFermiLATExceptionCurlInit,
    GRFermiLATExceptionCurlPerform,
    
    GRFermiLATExceptionFermiDataServerTooEarly,
    GRFermiLATExceptionFermiDataServerUnknown,
    
    GRFermiLATExceptionFermiDataServerQuiryStateUnknown,
    
    GRFermiLATExceptionFermiDataServerEmptyResults,
    GRFermiLATExceptionFermiDataServerUnknownFile,
    
    GRFermiLATExceptionNoEventListFile,
    GRFermiLATExceptionNoFilteredFile,
    GRFermiLATExceptionNoSpacecraftFile,
    GRFermiLATExceptionNoLtCubeFile,
    
    GRFermiLATExceptionGtmktimeFailed,
    
    GRFermiLATExceptionNoPhotons
    };

class GRFermiLAT {    
    static size_t saveFermiDataServerResponceToString(char *ptr, size_t size, size_t nmemb, string *string);
    static size_t saveFermiDataServerResponceToFile(char *ptr, size_t size, size_t nmemv, FILE *file);
    
    char digitHexCode(int digit);
    string hash(double startTime, double endTime, GRLocation location);
    
    string downloadPhotons(double startTime, double endTime, GRLocation location);
    void processPhotons(string queryHash);
    vector <GRFermiLATPhoton> allPhotons(double startTime, double endTime, GRLocation location);
    
    bool fileExists(string queryHash, string fileName);
    string instrumentResponceFunctionName(GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    string gtselect(string queryHash);
    string gtmktime(string queryHash);
    string gtltcube(string queryHash);
    string gtpsf(string queryHash, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    
    GRFermiEventClass eventClassFromPsfInt(int psfInt);
    GRFermiConversionType conversinoTypeFromPsfInt(int psfInt);
    
public:
    GRPsf psf(double startTime, double endTime, GRLocation location, GRFermiEventClass eventClass, GRFermiConversionType conversionType);
    vector <GRPhoton> photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRLocation location, float locationError, GRFermiEventClass worstEventClass, float confidence);
};

#endif /* defined(__Gamma_Rays__GRFermiLAT__) */
