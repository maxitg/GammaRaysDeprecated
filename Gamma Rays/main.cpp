//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include "GRBurst.h"

int main(int argc, const char * argv[])
{
    unsigned seed = (unsigned)time(NULL);
    cout << "seed : " << seed << endl;
    
    seed = 1367296752;
    
    srand(seed);
    
    GRBurst burst;
    /*
    burst.name = "GRB080916C";
    burst.time = 243216766.614;
    burst.location.ra = 119.85;
    burst.location.dec = -56.64;
    burst.location.error = 0.0001;
    burst.startOffset = 5.0;
    burst.endOffset = 209.8;
    */
    /*
    burst.name = "GRB090902B";
    burst.time = 273582310.313;
    burst.location.ra = 264.94;
    burst.location.dec = 27.324;
    burst.location.error = 0.001;
    burst.startOffset = 7.7;
    burst.endOffset = 825.0;
    */
    
    burst.name = "GRB090926A";
    burst.time = 275631628.990;
    burst.location.ra = 353.4;
    burst.location.dec = -66.32;
    burst.location.error = 0.01;
    burst.startOffset = 5.5;
    burst.endOffset = 225.0;
    
     
    burst.init();
    burst.download();
    burst.process();
    burst.read();
    burst.evaluate();
    
    ofstream probs("probs");
    for (int i = 0; i < burst.lengtheningValues.size(); i++) {
        probs << burst.lengtheningValues[i] << " " << burst.lengtheningProbabilities[i] << endl;
    }
    probs.close();
    
    //burst.name = "GRB080916C-1000";
    //burst.time = 243215766.614;
    //burst.location.ra = 119.85;
    //burst.location.dec = -56.64;
    //burst.location.error = 0.0001;
    //burst.startOffset = 5.0;
    //burst.endOffset = 209.8;
    /*
    for (int i = 0; i < 20; i++) {
        GRBurst burst;
        cout << "i = " << i << endl;
        burst.name = "BG";
        burst.time = ((double)rand())/RAND_MAX * (333803371.954 - 241366429.105) + 241366429.105;
        burst.location.ra = ((double)rand())/RAND_MAX * 360.;
        burst.location.dec = ((double)rand())/RAND_MAX * 180. - 90.;
        burst.location.error = ((double)rand())/RAND_MAX * 3.35;
        burst.startOffset = ((double)rand())/RAND_MAX * 52.2;
        burst.endOffset = burst.startOffset + ((double)rand())/RAND_MAX * (825.0 - 24.1) + 24.1;
        
        burst.init();
        cout << burst.query.hash << " " << burst.backgroundQuery.hash << endl;
        
        burst.download();
        burst.process();
        burst.read();
        burst.plotExposuredPhotons();
    }
     */
    return 0;
}