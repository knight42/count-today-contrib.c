#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <memory>

#include <ctime>
#include <cstring>

#include <curl/curl.h>

#define GITHUB_USER "knight42"
#define TIMEOUT 5

using std::string;
using std::cout;
using std::cerr;
using std::endl;

class Downloader {
public:
    Downloader () {
        curl = curl_easy_init();
        if (!curl) {
            cerr << "Failed to get CURL easy handle" << endl;
            exit(-1);
        }
        chunk = NULL;
    }
    ~Downloader () {
        curl_easy_cleanup(curl);
        curl_slist_free_all(chunk);
    }

    const string& download(const string &url, string tz) {
        size_t slash = tz.find('/');
        if (slash != string::npos) {
            tz.replace(slash, 1, "%2F");
        }
        CURLcode code;
        std::ostringstream cookie;
        cookie << "Cookie: tz=" << tz << ";";
        chunk = curl_slist_append(chunk, cookie.str().c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
    struct curl_slist *chunk;
    string buffer;

    static size_t write_data(char *ptr, size_t size, size_t nmemb, string *stream) {
        if (!stream) return 0;
        size_t len = size * nmemb;
        stream->append(ptr, len);
        return len;
    }
};


typedef std::istringstream sstream;

sstream exec(const char* cmd) {
    const size_t bufsiz = 256;
    std::array<char, bufsiz> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), bufsiz, pipe.get()) != NULL)
            result += buffer.data();
    }
    return sstream(result);
}

string getTZ() {
    // Sample output:
    // ...
    //        Time zone: Asia/Shanghai (CST, +0800)
    // ...
    sstream output = exec("timedatectl");
    string line;
    string res("");
    while(std::getline(output, line)) {
        if (line.find("Time zone") == string::npos) {
            continue;
        }
        size_t colon = line.find(':');
        size_t bracket = line.find('(');
        if (colon == string::npos || bracket == string::npos) {
            break;
        }
        res = line.substr(colon + 2, bracket - colon - 3);
    }
    return res;
}

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
    string data(d.download(url, getTZ()));
    if (data.empty()) {
        return -1;
    }

    sstream iss(data);

    const char *pat = "data-count=";
    const char *cnt = "0";
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
            cnt = num.c_str();
        }
    }
    cout << cnt << endl;
    return 0;
}
