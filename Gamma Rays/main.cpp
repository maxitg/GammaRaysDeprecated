//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <iostream>

#include "GRFermiLATPhoton.h"

int main(int argc, const char * argv[])
{
    GRFermiLATPhoton photon = GRFermiLATPhoton(time(NULL), 12.34, 53.2, 1e6, GRFermiConversionTypeFront, GRFermiEventClassClean);
    GRPhoton usualPhoton = GRPhoton(time(NULL), 12.34, 53.2, 1e5);
    cout << usualPhoton.description();
    
    return 0;
}