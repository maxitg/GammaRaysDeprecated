//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <iostream>

#include "GRBurst.h"

int main(int argc, const char * argv[])
{
    GRBurst burst = GRBurst("GRB080916C", 243216766, 119.88, -56.59);
    vector <GRPhoton> photons = burst.photons();
    for (vector <GRPhoton>::iterator i = photons.begin(); i != photons.end(); i++) {
        cout << i->description() << endl;
    }
    return 0;
}