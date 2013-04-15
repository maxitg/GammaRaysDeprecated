//
//  GRDistribution.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 07.04.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <math.h>
#include <cmath>

#include <fstream>

#include <glpk.h>

#include "GRDistribution.h"

int GRDistribution::size() {
    return values.size();
}

vector <GRDistributionCDFPoint> GRDistribution::cdf() {
    vector <GRDistributionCDFPoint> result;
    result.reserve(2*values.size());
    for (int i = 0; i < values.size(); i++) {
        GRDistributionCDFPoint point;
        point.value = values[i];
        point.probability = (double)i/values.size();
        result.push_back(point);
        point.value = values[i];
        point.probability = (double)(i+1)/values.size();
        result.push_back(point);
    }
    return result;
}

double GRDistribution::kolmogorovSmirnovProbability(double distance, int n1, int n2) {
    double n = (double)n1*n2/(n1+n2);
    double z = (sqrt(n) + 0.12 + 0.11/sqrt(n)) * distance;
    
    if (z == 0.) return 1.;
    if (z < 1.18) {
        double y = exp(-1.23370055013616983/pow(z,2.));
        return 1. - 2.25675833419102515*sqrt(-log(y))*(y + pow(y,9) + pow(y,25) + pow(y,49));
    } else {
        double x = exp(-2.*pow(z,2.));
        return 2.*(x - pow(x,4) + pow(x,9));
    }
}

double GRDistribution::kolmogorovSmirnovDistance(double probability, int n1, int n2) {
    double y,logy,yp,x,xp,f,ff,u,t;
    double z;
    if (probability == 1.) z = 0.;
    else if (probability > 0.3) {
        f = -0.392699081698724155*pow(1.-probability, 2.);
        
        const double ooe = 0.367879441171442322;
        double tp,up,to=0.;
        if (f < -0.2) up = log(ooe-sqrt(2*ooe*(f+ooe)));
        else up = -10.;
        do {
            up += (tp=(log(f/up)-up)*(up/(1.+up)));
            if (tp < 1.e-8 && abs(tp+to)<0.01*abs(tp)) break;
            to = tp;
        } while (abs(tp/up) > 1.e-15);
        y = exp(up);
        
        do {
            yp = y;
            logy = log(y);
            ff = f/pow(1.+ pow(y,4)+ pow(y,12), 2.);
            u = (y*logy-ff)/(1.+logy);
            y = y - (t=u/max(0.5,1.-0.5*u/(y*(1.+logy))));
        } while (abs(t/y)>1.e-15);
        z = 1.57079632679489662/sqrt(-log(y));
    } else {
        x = 0.03;
        do {
            xp = x;
            x = 0.5*probability+pow(x,4)-pow(x,9);
            if (x > 0.06) x += pow(x,16)-pow(x,25);
        } while (abs((xp-x)/x)>1.e-15);
        z = sqrt(-0.5*log(x));
    }
    double n = (double)n1*n2/(n1+n2);
    return z / (sqrt(n) + 0.12 + 0.11/sqrt(n));
}

float GRDistribution::kolmogorovSmirnovTest(GRDistribution distribution, double shift, double lengthening) {
    for (int i = 0; i < distribution.values.size(); i++) {
        distribution.values[i] += shift;
        distribution.values[i] *= lengthening;
    }
    
    std::ofstream dataMeV("dataMev");
    std::ofstream dataGeV("dataGeV");
    
    for (int i = 0; i < values.size(); i++) {
        dataMeV << values[i] << " " << (double)i/values.size() << endl;
    }
    for (int i = 0; i < distribution.values.size(); i++) {
        dataGeV << distribution.values[i] << " " << (double)i/distribution.values.size() << endl;
    }
    
    dataMeV.close();
    dataGeV.close();
    
    double maxDistance = 0.;
    int index;
    for (int i = 0; i < distribution.values.size(); i++) {
        vector <double>::iterator lowerBound = lower_bound(values.begin(), values.end(), distribution.values[i]);
        double thisIndex = distance(values.begin(), lowerBound);
        double thisHeight = thisIndex / values.size();
        
        double heightLeft = (double)i / distribution.values.size();
        double heightRight = (double)(i+1) / distribution.values.size();
        
        double distanceLeft = thisHeight - heightLeft;
        if (distanceLeft < 0) distanceLeft = -distanceLeft;
        double distanceRight = thisHeight - heightRight;
        if (distanceRight < 0) distanceRight = -distanceRight;
        
        if (distanceLeft > maxDistance) {
            index = i;
            maxDistance = distanceLeft;
        }
        if (distanceRight > maxDistance) {
            index = i;
            maxDistance = distanceRight;
        }
    }
        
    return kolmogorovSmirnovProbability(maxDistance, values.size(), distribution.values.size());
}

double GRDistribution::parameterLimit(GRDistribution distribution, float probability, GRDistributionParameter parameter, GRDistributionObjective objective, bool *success, bool allowShift, bool allowLengthening) {
    int ia[2*values.size()+1];
    int ja[2*values.size()+1];
    double ar[2*values.size()+1];
    
    glp_prob *problem = glp_create_prob();
    glp_set_obj_dir(problem, objective == GRDistributionObjectiveMaximize ? GLP_MIN : GLP_MAX);
    glp_term_out(0);
    
    glp_add_rows(problem, values.size());
    glp_add_cols(problem, 2);
    
    if (allowLengthening) {
        glp_set_col_bnds(problem, 1, GLP_LO, 0, 0);
    } else {
        glp_set_col_bnds(problem, 1, GLP_FX, 1., 1.);
    }
    
    if (parameter == GRDistributionParameterLengthening) {
        glp_set_obj_coef(problem, 1, 1.);
    } else {
        glp_set_obj_coef(problem, 1, 0.);
    }
    
    if (allowShift) {
        glp_set_col_bnds(problem, 2, GLP_FR, 0, 0);
    } else {
        glp_set_col_bnds(problem, 2, GLP_FX, 0., 0.);
    }
    
    if (parameter == GRDistributionParameterShift) {
        glp_set_obj_coef(problem, 2, 1.);
    } else {
        glp_set_obj_coef(problem, 2, 0.);
    }
    
    double maxKolmogorovSmirnovDistance = kolmogorovSmirnovDistance(probability, distribution.values.size(), values.size());
    
    for (int i = 0; i < values.size(); i++) {
        double lowerCDF, upperCDF;
        lowerCDF = (double)(i+1)/values.size() - maxKolmogorovSmirnovDistance;
        upperCDF = (double)i/values.size() + maxKolmogorovSmirnovDistance;
        
        int leftIndex = floor(lowerCDF*distribution.values.size());
        int rightIndex = floor(upperCDF*distribution.values.size());
        
        if (lowerCDF <= 0. && upperCDF >= 1.) glp_set_row_bnds(problem, i+1, GLP_FR, 0., 0.);
        else if (lowerCDF <= 0. && upperCDF < 1.) glp_set_row_bnds(problem, i+1, GLP_UP, 0., distribution.values[rightIndex]);
        else if (lowerCDF > 0. && upperCDF >= 1.) glp_set_row_bnds(problem, i+1, GLP_LO, distribution.values[leftIndex], 0.);
        else glp_set_row_bnds(problem, i+1, GLP_DB, distribution.values[leftIndex], distribution.values[rightIndex]);
        
        ia[2*i+1] = i+1;
        ia[2*i+2] = i+1;
        ja[2*i+1] = 1;
        ja[2*i+2] = 2;
        
        ar[2*i+1] = values[i];
        ar[2*i+2] = 1.;
        
        //cout << (leftIndex >= 0 ? distribution.values[leftIndex] : -INFINITY) << " < " << values[i] << " * len + 1 * shift < " << (rightIndex < distribution.values.size() ? distribution.values[rightIndex] : INFINITY) << endl;
    }
    
    glp_load_matrix(problem, 2*values.size(), ia, ja, ar);
    glp_simplex(problem, NULL);
    
    if (success) {
        if (glp_get_status(problem) == GLP_NOFEAS) *success = false;
        else *success = true;
    }
    
    double lengthening = 1./glp_get_col_prim(problem, 1);
    double shift = -glp_get_col_prim(problem, 2);
    
    glp_delete_prob(problem);
    glp_free_env();
        
    return parameter == GRDistributionParameterShift ? shift : lengthening;
}