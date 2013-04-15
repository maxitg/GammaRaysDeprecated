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
#include "GRBurstStorage.h"

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
    } else {
        out << "impossible" << endl;
    }
}

void drawPicture(GRBurst burst, double probability, string filename, bool chat = false, double lengtheningSteps = 100, double shiftSteps = 100) {
    ofstream dens(filename.c_str());
    bool success;
    double minL = burst.minLengtheningAllowed(probability, &success);
    double maxL = burst.maxLengtheningAllowed(probability, &success);
    double sprL = exp((log(maxL) - log(minL)) * 0.1);
    minL = minL/sprL;
    maxL = maxL*sprL;
    double stepL = (maxL - minL)/lengtheningSteps;
    double minS = burst.minShiftAllowed(probability, &success);
    double maxS = burst.maxShiftAllowed(probability, &success);
    double sprS = (maxS - minS) * 0.1;
    minS = minS - sprS;
    maxS = maxS + sprS;
    double stepS = (maxS - minS)/shiftSteps;
    
    if (!success) {
        if (chat) cout << "too low to draw" << endl;
        return;
    }
    
    for (double lengthening = minL; lengthening <= maxL; lengthening += stepL) {
        if (chat) cout << "drawing " << lengthening << " ..." << endl;
        for (double shift = minS; shift <= maxS; shift += stepS) {
            double prob = burst.gevTransformHypothesisProbability(shift, lengthening);
            dens << lengthening << " " << shift << " " << (prob > probability ? prob : 0) << endl;
        }
    }
    dens.close();
}

void drawCDF(GRBurst burst, float minEnergy, float maxEnergy, string filename) {
    ofstream cdfFile(filename.c_str());
    vector <GRDistributionCDFPoint> points = burst.photonDistributionFromStart(minEnergy, maxEnergy).cdf();
    for (int i = 0; i < points.size(); i++) {
        cdfFile << points[i].value << " " << points[i].probability << endl;
    }
    cdfFile.close();
}

int main(int argc, const char * argv[])
{
    
    GRBurstStorage *storage;
    storage->getInstance();
    vector <GRBurst> burstCatalog = storage->bursts(0., 387639860.000000);
    for (int i = 0; i < burstCatalog.size(); i++) {
        cout << burstCatalog[i].description() << endl;
    }
    cout << fixed << 387639860.000000 << endl;
    
    return 0;
    
    ifstream bursts("bursts");
    ofstream log("log");
    
    int n;
    bursts >> n;
    
    for (int i = 0; i < n; i++) {
        string name;
        double MET;
        float ra;
        float dec;
        float locationError;
        
        bursts >> name >> MET >> ra >> dec >> locationError;
    
        cout << "processing " << name << "..." << endl;

        GRLocation location = GRLocation(GRCoordinateSystemJ2000, ra, dec);
        GRBurst burst = GRBurst(name, MET, 0., 0., location, locationError);
    
        log << burst.name << " : " << burst.gevCount() << " GeV photons" << endl;
        if (burst.gevCount() < 10) continue;
        
        drawCDF(burst, 0., 1000., burst.name + ".MeV.cdf");
        drawCDF(burst, 1000., INFINITY, burst.name + ".GeV.cdf");
        
        printRanges(burst, 0.05, log);
        drawPicture(burst, 0.05, burst.name + ".2s.pict", true);
        log << endl;
        printRanges(burst, 0.32, log);
        drawPicture(burst, 0.32, burst.name + ".1s.pict", true);
        log << endl;
    }
    
    log.close();
    bursts.close();
     
    return 0;
}