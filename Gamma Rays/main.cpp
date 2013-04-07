//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <iostream>

#include "GRBurst.h"
#include "GRFermiLAT.h"
#include "GRPhotonStorage.h"

int main(int argc, const char * argv[])

{
    GRBurst burst = GRBurst("GRB100728095", 301976252, GRCoordinateSystemGalactic, 220.663f, -19.2164f);
    
    GRFermiLAT fermiLAT;
    GRLocation location(GRCoordinateSystemGalactic, 220.663f, -19.2164f);
    
    cout << location.ra << " " << location.dec << endl;
    
    GRPsf psf = fermiLAT.psf(275631628-500, 275631628+2000, location, GRFermiEventClassSource, GRFermiConversionTypeFront);
    cout << psf.spread(100.f, 0.95) << endl;
    
    GRPhotonStorage *storage;
    storage->getInstance();
    vector <GRPhoton> photons = storage->photons(275631628-500, 275631628+2000, 0, 100000, location);
    
    for (int i = 0; i < photons.size(); i++) {
        cout << i+1 << ": " << photons[i].description() << endl;
    }
    
    return 0;
}