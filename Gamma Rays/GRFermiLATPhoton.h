//
//  GRFermiLATPhoton.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRFermiLATPhoton__
#define __Gamma_Rays__GRFermiLATPhoton__

#include <iostream>
#include <string>

#include "GRPhoton.h"

enum GRFermiConversionType {
    GRFermiConversionTypeFront = 0,
    GRFermiConversionTypeBack = 1
    };

enum GRFermiEventClass {
    GRFermiEventClassTransient = 0,
    GRFermiEventClassSource = 1,
    GRFermiEventClassClean = 2,
    GRFermiEventClassUltraclean = 3
    };

class GRFermiLATPhoton : GRPhoton {
    GRFermiConversionType conversionType;
    GRFermiEventClass eventClass;
    
    string conversionTypeName();
    string eventClassName();
    
public:
    GRFermiLATPhoton(double time, float ra, float dec, float energy, GRFermiConversionType conversionType, GRFermiEventClass fermiEventClass) : GRPhoton(time, ra, dec, energy), conversionType(conversionType), eventClass(fermiEventClass) {};
    
    string description();
};

#endif /* defined(__Gamma_Rays__GRFermiLATPhoton__) */
