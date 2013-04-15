//
//  GRBurstStorage.h
//  Gamma Rays
//
//  Created by Maxim Piskunov on 14.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#ifndef __Gamma_Rays__GRBurstStorage__
#define __Gamma_Rays__GRBurstStorage__

#include <iostream>
#include <vector>

#include "GRBurst.h"

using namespace std;

class GRBurstStorage {
public:
    static GRBurstStorage& getInstance() {
        static GRBurstStorage instance;
        return instance;
    }
    
private:
    GRBurstStorage() {};
    GRBurstStorage(GRBurstStorage const&);
    void operator=(GRBurstStorage const&);
    
public:
    vector <GRBurst> bursts(double startTime, double endTime);
};

#endif /* defined(__Gamma_Rays__GRBurstStorage__) */
