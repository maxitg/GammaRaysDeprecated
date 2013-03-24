//
//  GRFermiLAT.cpp
//  Gamma Rays
//
//  Created by Maxim Piskunov on 24.03.2013.
//  Copyright (c) 2013 Maxim Piskunov. All rights reserved.
//

#include <sstream>

#include "GRFermiLAT.h"
#include <curl/curl.h>

size_t my_dummy_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    cout << ptr;
    return size * nmemb;
}

vector<GRFermiLATPhoton> GRFermiLAT::photons(double startTime, double endTime, float minEnergy, float maxEnergy, GRCelestialSpherePoint location, GRFermiEventClass worstEventClass) {
    
    vector <GRFermiLATPhoton> result;
    
    /*  
        <form method="post" action="/cgi-bin/ssc/LAT/LATDataQuery.cgi" enctype="multipart/form-data">
            <input value="query" name="destination" type="hidden" />
            <input value="Start Search" type="submit" />
            <input value="Reset" type="reset" />
            <table>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#objectNameOrCoordinates"><b>Object name or coordinates:</b></a></td><td><input value name="coordfield" type="text" /></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#coordinateSystem"><b>Coordinate system:</b></a></td><td><select name="coordsystem"><option>J2000</option><option>B1950</option><option>Galactic</option></select></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#searchRadius"><b>Search radius (degrees):</b></a></td><td><input value name="shapefield" type="text" /></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#observationDates"><b>Observation dates:</b></a></td><td><input value name="timefield" type="text" /></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#timeSystem"><b>Time system:</b></a></td><td><select name="timetype"><option>Gregorian</option><option>MET</option><option>MJD</option></select></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#energyRange"><b>Energy range (MeV):</b></a></td><td><input value name="energyfield" type="text" /></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#LATdataType"><b>LAT data type:</b></a></td><td><select name="photonOrExtendedOrNone"><option>Photon</option><option>Extended</option><option>None</option></select></td></tr>
                <tr><td><a target="HelpWindow" href="http://fermi.gsfc.nasa.gov/ssc/LATDataQuery_help.html#spacecraftData"><b>Spacecraft data:</b></a></td><td><input checked="checked" name="spacecraft" id="spacecraft" type="checkbox" /></td></tr>
            </table>
        </form>
    */

    ostringstream coordfield;
    coordfield << location.ra << ", " << location.dec;
    const char *coordfield_str = coordfield.str().c_str();
    
    ostringstream shapefield;
    shapefield << 40.;
    const char *shapefield_str = shapefield.str().c_str();
    
    ostringstream timefield;
    timefield << startTime << ", " << endTime;
    const char *timefield_str = timefield.str().c_str();
    
    ostringstream energyfield;
    energyfield << minEnergy << ", " << maxEnergy;
    const char *energyfield_str = energyfield.str().c_str();
    
    const char *photonOrExtendedOrNone_str = (worstEventClass == GRFermiEventClassTransient ? "Extended" : "Photon");
    
    CURL *curl;
    CURLcode res;
    
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "destination",
                 CURLFORM_COPYCONTENTS, "query",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "coordfield",
                 CURLFORM_COPYCONTENTS, coordfield_str,
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "coordsystem",
                 CURLFORM_COPYCONTENTS, "J2000",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "shapefield",
                 CURLFORM_COPYCONTENTS, shapefield_str,
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "timefield",
                 CURLFORM_COPYCONTENTS, timefield_str,
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "timetype",
                 CURLFORM_COPYCONTENTS, "MET",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "energyfield",
                 CURLFORM_COPYCONTENTS, energyfield_str,
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "photonOrExtendedOrNone",
                 CURLFORM_COPYCONTENTS, photonOrExtendedOrNone_str,
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "spacecraft",
                 CURLFORM_COPYCONTENTS, "on",
                 CURLFORM_END);
    
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "send",
                 CURLFORM_END);
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://fermi.gsfc.nasa.gov/cgi-bin/ssc/LAT/LATDataQuery.cgi");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &my_dummy_write);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
    }
    
    return result;
}