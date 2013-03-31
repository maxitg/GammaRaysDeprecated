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

int main(int argc, const char * argv[])
{
    GRBurst burst = GRBurst("GRB100728095", 301976252, GRCoordinateSystemGalactic, 220.663, -19.2164);
    
    GRFermiLAT fermiLAT;
    GRLocation location(GRCoordinateSystemGalactic, 220.663, -19.2164);
    cout << location.description();
    
    //fermiLAT.downloadPhotons(301976252-500, 301976252+2000, location);
    
    return 0;
}