#ifndef UTILITY_H
#define UTILITY_H

#ifdef __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
#endif

#include <random>
#include <math.h>
#include <iostream>
#include <string>

#include <curl/curl.h>

namespace Utility
{
    std::string getBasePath(void);
	std::string doWebRequest(std::string url);
};

#endif