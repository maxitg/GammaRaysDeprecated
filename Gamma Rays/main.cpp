//
//  main.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>
#include <iostream>

#include "GRBurst.h"
#include "GRFermiLAT.h"
#include "GRPhotonStorage.h"

void printRanges(GRBurst burst, double prob, ostream &out) {
    bool success;
    
    out << "ranges for probability: " << prob << endl;
    out << "everything allowed: " << endl;
    double minShift = burst.minShiftAllowed(prob, &success, true);
    double maxShift = burst.maxShiftAllowed(prob, &success, true);
    double minLengthening = burst.minLengtheningAllowed(prob, &success, true);
    double maxLengthening = burst.maxLengtheningAllowed(prob, &success, true);
    if (success) {
        out << "shift range: (" << minShift << ", " << maxShift << ")" << endl;
        out << "lengthening range: (" << minLengthening << ", " << maxLengthening << ")" << endl;
    } else {
        out << "impossible" << endl;
    }
    out << endl;
    
    out << "only shift allowed: " << endl;
    minShift = burst.minShiftAllowed(prob, &success, false);
    maxShift = burst.maxShiftAllowed(prob, &success, false);
    if (success) {
        out << "shift range: (" << burst.minShiftAllowed(prob, &success, false) << ", " << burst.maxShiftAllowed(prob, &success, false) << ")" << endl;
    } else {
        out << "impossible" << endl;
    }
    out << endl;
    
    out << "only lengthening allowed: " << endl;
    minLengthening = burst.minLengtheningAllowed(prob, &success, false);
    maxLengthening = burst.maxLengtheningAllowed(prob, &success, false);
    if (success) {
        out << "lengthening range: (" << burst.minLengtheningAllowed(prob, &success, false) << ", " << burst.maxLengtheningAllowed(prob, &success, false) << ")" << endl;
        out << burst.gevTransformHypothesisProbability(0., minLengthening) << endl;
    } else {
        out << "impossible" << endl;
    }
}

void drawPicture(GRBurst burst, double probability, string filename, bool chat = false, double lengtheningStep = 1.01, double shiftStep = 0.1) {
    ofstream dens(filename.c_str());
    double minL = burst.minLengtheningAllowed(probability, NULL);
    double maxL = burst.maxLengtheningAllowed(probability, NULL);
    double sprL = exp((log(maxL) - log(minL)) * 0.1);
    double minS = burst.minShiftAllowed(probability, NULL);
    double maxS = burst.maxShiftAllowed(probability, NULL);
    double sprS = (maxS - minS) * 0.1;
    for (double lengthening = minL/sprL; lengthening <= maxL*sprL; lengthening *= lengtheningStep) {
        for (double shift = minS - sprS; shift <= maxS + sprS; shift += shiftStep) {
            double prob = burst.gevTransformHypothesisProbability(shift, lengthening);
            if (chat) cout << lengthening << " " << shift << " " << prob << endl;
            dens << lengthening << " " << shift << " " << (prob > probability ? prob : 0) << endl;
        }
    }
    dens.close();
}

int main(int argc, const char * argv[])

{
    ifstream bursts("bursts");
    ofstream log("log");
    
    int n;
    bursts >> n;
    
    for (int i = 0; i < n; i++) {
        string name;
        double MET;
        float ra;
        float dec;
        
        bursts >> name >> MET >> ra >> dec;
    
        cout << "processing " << name << "..." << endl;

        GRBurst burst = GRBurst(name, MET, GRCoordinateSystemJ2000, ra, dec);
    
        log << burst.name << " : " << burst.gevCount() << " GeV photons" << endl;
        if (burst.gevCount() < 10) continue;
        
        printRanges(burst, 0.05, log);
        drawPicture(burst, 0.05, burst.name + ".2s.pict", false, 1.1, 0.1);
        log << endl;
        printRanges(burst, 0.32, log);
        drawPicture(burst, 0.32, burst.name + ".1s.pict", false, 1.1, 0.1);
        log << endl;
    }
    
    log.close();
    bursts.close();
     
    return 0;
}