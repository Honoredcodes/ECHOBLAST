#include <iostream>
#include <string>
#include <cstring>
#include <curl/curl.h>

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* data = static_cast<std::string*>(userdata);
    if (data->empty()) return 0;

    size_t toCopy = std::min(size * nmemb, data->size());
    memcpy(ptr, data->data(), toCopy);
    data->erase(0, toCopy);
    return toCopy;
}

void SetCurlForMail(CURL*& curl, std::string servername, int port, std::string username, std::string password) {
    std::string protocol = (port == 465) ? "smtps://" : "smtp://";
    std::string url = protocol + servername + ":" + std::to_string(port);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // for debugging
}

// Function to perform the email sending operation using CURL
void PerformCurlSend(CURL* curl, CURLcode& res, const std::string& sender, const std::string& lead, void* emailBody, curl_read_callback readCallback, long& responseCode) {
    struct curl_slist* recipients = curl_slist_append(nullptr, lead.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sender.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READDATA, emailBody);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_slist_free_all(recipients);
}