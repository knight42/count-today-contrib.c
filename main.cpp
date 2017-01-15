#include <iostream>
#include <sstream>
#include <string>

#include <ctime>
#include <cstring>

#include <curl/curl.h>

#define GITHUB_USER "knight42"
#define TIMEOUT 5

using std::string;
using std::cout;
using std::endl;

class Downloader {
public:
    Downloader () {
        curl = curl_easy_init();
    }
    ~Downloader () {
        curl_easy_cleanup(curl);
    }

    const string& download(const string &url) {
        CURLcode code;
        code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
        code = curl_easy_perform(curl);
        if (code != CURLE_OK) {
            buffer.clear();
        }
        return buffer;
    }

private:
    CURL *curl;
    string buffer;

    static size_t write_data(char *ptr, size_t size, size_t nmemb, string *stream) {
        if (!stream) return 0;
        size_t len = size * nmemb;
        stream->append(ptr, len);
        return len;
    }
};


int main(int argc, char *argv[]) {
    char url[100];
    const char *username = NULL;
    if (argc > 1) {
        username = argv[1];
    } else {
        username = GITHUB_USER;
    }
    sprintf(url, "https://github.com/users/%s/contributions", username);

    char date[16];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(date, 16, "%F", tm);

    Downloader d;
    std::istringstream iss(d.download(url));

    const char *pat = "data-count=";
    for (string line; std::getline(iss, line); ) {
        if (line.find(date) == string::npos) {
            continue;
        }
        size_t pos = line.find(pat);
        if (pos != string::npos) {
            pos += strlen(pat) + 1; // skip "
            size_t begin = pos;

            for (char c = line.at(pos);
                 c >= '0' && c <= '9';
                 c = line.at(++pos));

            string num = line.substr(begin, pos - begin);
            cout << num << endl;
        }
    }
    return 0;
}
