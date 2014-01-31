#include "Utility.h"

std::string Utility::getBasePath(void)
{
    std::string path = "../";
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char p[PATH_MAX];
    if(!CFURLGetFileSystemRepresentation(resourceURL, TRUE, (UInt8 *)p, PATH_MAX))
        std::cout << "ERROROROROR" << std::endl;
    CFRelease(resourceURL);
    
    chdir(p);
    
    path = p + std::string("/../../../");
#endif
    return path;
}

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
	((std::string*)stream)->append((char*)ptr, 0, size * count);
	return size * count;
}

std::string Utility::doWebRequest(std::string url)
{
	CURL* curl_handle = NULL;
	std::string response;

	/* initializing curl and setting the url */
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

	/* follow locations specified by the response header */
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

	/* setting a callback function to return the data */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeToString);

	/* passing the pointer to the response as the callback parameter */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

	/* perform the request */
	curl_easy_perform(curl_handle);

	/* cleaning all curl stuff */
	curl_easy_cleanup(curl_handle);

	return response;
}