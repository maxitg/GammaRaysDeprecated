//
//  GRFermiLATPhoton.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <sstream>

#include "GRFermiLATPhoton.h"

string GRFermiLATPhoton::conversionTypeName() {
    if (conversionType == GRFermiConversionTypeFront) return "front";
    else return "back";
}

string GRFermiLATPhoton::eventClassName() {
    if (eventClass == GRFermiEventClassUltraclean) return "ultraclean";
    else if (eventClass == GRFermiEventClassClean) return "clean";
    else if (eventClass == GRFermiEventClassSource) return "source";
    else return "transient";
}

string GRFermiLATPhoton::description() {
    ostringstream result;
    result << "Photon (" << energyDescription() << ", " << eventClassName() << ", " << conversionTypeName() << ") at MET " << (int)time << " from " << location.description();
    return result.str();
}