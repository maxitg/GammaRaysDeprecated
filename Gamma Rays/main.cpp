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
    GRBurst burst = GRBurst("GRB090926A", 275631628, GRCoordinateSystemJ2000, 353.56f, -66.34f);
    vector <GRPhoton> photons = burst.photons();
    
    for (int i = 0; i < photons.size(); i++) {
        cout << i+1 << ": " << photons[i].description() << endl;
    }
    
    cout << "0.05 event time: " << fixed << burst.passTimeOfPhotonsFraction(0.05) << endl;
    cout << "official MET is: " << 275631628 << endl;
    
    cout << burst.gevTransformHypothesisProbability(0, 0.6) << endl;
    
    return 0;
}