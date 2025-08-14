#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <iostream>

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
void SetCurlForMail(CURL*& curl, std::string servername, int port, std::string username, std::string password);
void PerformCurlSend(CURL* curl, CURLcode& res, const std::string& sender, const std::string& lead, void* emailBody, curl_read_callback readCallback, long& responseCode);
#endif