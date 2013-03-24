//
//  GRCelestialSpherePoint.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <sstream>

#include "GRCelestialSpherePoint.h"

string GRCelestialSpherePoint::description() {
    ostringstream result;
    result << "(" << ra << ", " << dec << ")";
    return result.str();
}