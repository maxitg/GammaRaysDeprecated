//
//  GRPsf.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 31.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <memory>
#include <sstream>

#include <fitsio.h>

#include "GRPsf.h"

using namespace std;

GRPsf::GRPsf(string filename) {
    fitsfile *psfFile;
    int status = 0;
    long nrows;
    
    fits_open_table(&psfFile, filename.c_str(), READONLY, &status);
    
    fits_movabs_hdu(psfFile, 3, NULL, &status);
    fits_get_num_rows(psfFile, &nrows, &status);
    float readAngles[nrows];
    fits_read_col(psfFile, TFLOAT, 1, 1, 1, nrows, 0, readAngles, 0, &status);
    angles.resize(nrows);
    for (int i = 0; i < nrows; i++) angles[i] = readAngles[i];
    
    fits_movabs_hdu(psfFile, 2, NULL, &status);
    fits_get_num_rows(psfFile, &nrows, &status);
    float readEnergies[nrows];
    fits_read_col(psfFile, TFLOAT, 1, 1, 1, nrows, 0, readEnergies, 0, &status);
    energies.resize(nrows);
    for (int i = 0; i < nrows; i++) energies[i] = readEnergies[i];
    
    probabilityDensity.resize(energies.size());
    for (int i = 0; i < energies.size(); i++) {
        float readProbabilityDensities[nrows];
        fits_read_col(psfFile, TFLOAT, 3, i+1, 1, angles.size(), 0, readProbabilityDensities, 0, &status);
        probabilityDensity[i].resize(angles.size());
        for (int j = 0; j < angles.size(); j++) probabilityDensity[i][j] = readProbabilityDensities[j];
    }
    
    fits_close_file(psfFile, &status);
    
    if (status) fits_report_error(stderr, status);
}

string GRPsf::description() {
    ostringstream result;
    result << "angles: ";
    for (int i = 0; i < angles.size(); i++) {
        result << angles[i] << " ";
    }
    result << endl;
    
    result << "energies: ";
    for (int i = 0; i < energies.size(); i++) {
        result << energies[i] << " ";
    }
    result << endl;
    
    result << "probability densities: ";
    result << endl;
    for (int i = 0; i < probabilityDensity.size(); i++) {
        for (int j = 0; j < probabilityDensity[i].size(); j++) {
            result << probabilityDensity[i][j] << " ";
        }
        result << endl;
    }
    
    return result.str();
}