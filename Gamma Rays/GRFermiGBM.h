//
//  GRFermiGBM.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 14.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRFermiGBM__
#define __Gamma_Rays__GRFermiGBM__

#include <iostream>

#include "GRBurst.h"

class GRFermiGBM {
    double latestDownloadTime();
    void download();
    
    double currentTime();
    double timeFromUnixTime(double unixTime);
    
public:
    vector <GRBurst> bursts(double startTime, double endTime);
};

#endif /* defined(__Gamma_Rays__GRFermiGBM__) */
