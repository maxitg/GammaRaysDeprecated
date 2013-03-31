//
//  GRPsf.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 31.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <memory>

#include <CCfits/CCfits.h>
#include <CCfits/FITS.h>
#include <CCfits/Column.h>

#include "GRPsf.h"

using namespace CCfits;
using namespace std;

GRPsf::GRPsf(string filename) {
    FITS fitsfile(filename, Read, true);
    cout << fitsfile.extension("THETA") << endl;
}

string GRPsf::description() {
    return "It's a psf";
}