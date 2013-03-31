//
//  GRPsf.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 31.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRPsf__
#define __Gamma_Rays__GRPsf__

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class GRPsf {
    vector <float> energies;
    vector <float> angles;
    vector <vector <float> > probabilityDensity;
    
public:
    GRPsf(string fileName);
    string description();
};

#endif /* defined(__Gamma_Rays__GRPsf__) */
