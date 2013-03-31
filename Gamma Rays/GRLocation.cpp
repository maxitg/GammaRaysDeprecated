//
//  GRCelestialSpherePoint.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>

#include <sstream>

#include "GRLocation.h"

GRLocation::GRLocation(GRCoordinateSystem system, float angle0, float angle1) {
    if (system == GRCoordinateSystemJ2000) {
        ra = angle0;
        dec = angle1;
    } else if (system ==GRCoordinateSystemGalactic) {
        double l = angle0 * M_PI / 180.;
        double b = angle1 * M_PI / 180.;
        
        double pole_ra = 192.859508 * M_PI / 180.;
        double pole_dec = 27.128336 * M_PI / 180.;
        double posangle = (122.932-90.0) * M_PI / 180.;
        
        double ra_rad = atan2( (cos(b)*cos(l-posangle)), (sin(b)*cos(pole_dec) - cos(b)*sin(pole_dec)*sin(l-posangle)) ) + pole_ra;
        double dec_rad = asin( cos(b)*cos(pole_dec)*sin(l-posangle) + sin(b)*sin(pole_dec) );
        
        ra = ra_rad * 180. / M_PI;
        dec = dec_rad * 180. / M_PI;
    }
}

string GRLocation::description() {
    ostringstream result;
    result << "(" << ra << " ra, " << dec << " dec)";
    return result.str();
}