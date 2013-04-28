//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include "GRFermiLATDataServerQuery.h"

int main(int argc, const char * argv[])
{    
    GRFermiLATDataServerQuery query;
    query.startTime = 243216766.614 + 5.0;
    query.endTime = 243216766.614 + 209.8;
    query.location.ra = 119.85;
    query.location.dec = -56.64;
    query.location.error = 0.0001;
    
    query.init();
    query.download();
    query.process();
    query.read();
    
    cout << query.exposureMaps[0][0].exposure(1000.f, query.location) << endl;
    cout << query.error << endl;
    return 0;
}