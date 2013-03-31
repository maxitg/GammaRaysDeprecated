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
    GRBurst burst = GRBurst("GRB100728095", 301976252, GRCoordinateSystemGalactic, 220.663f, -19.2164f);
    
    GRFermiLAT fermiLAT;
    GRLocation location(GRCoordinateSystemGalactic, 220.663f, -19.2164f);
    
    fermiLAT.psf(275631628-500, 275631628+2000, location, GRFermiEventClassTransient, GRFermiConversionTypeBack);
    
    return 0;
}