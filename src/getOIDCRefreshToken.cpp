#include "main.hpp"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_code_from_headers(std::string const &str) {
    std::stringstream ss(str);
    std::string line;
    while (ss&&!ss.eof()&&getline(ss,line)) {
      auto const p = line.find("Location:");
      if (p!=std::string::npos) {
        auto const p = line.find("=");
        return line.substr(p+1);
      }
    }
    return "";
}

Aws::String getOIDCRefreshToken(const std::string &IAMHost, const std::string &clientId, const std::string &clientSecret,
                                const std::string &certFile, const std::string &keyFile, const std::string &cookiesFile)
{
  CURL *curl;
  CURLcode res;
  std::string readBufferDiscovery;
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  auto curl_config_url = IAMHost + "/.well-known/openid-configuration";
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, curl_config_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBufferDiscovery);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed at openid configuration discovery: %s\n",
              curl_easy_strerror(res));
      curl_easy_cleanup(curl);
      return "";
    }
    curl_easy_cleanup(curl);
  }
  if (nlohmann::json::accept(readBufferDiscovery)) {
    nlohmann::json data = nlohmann::json::parse(readBufferDiscovery);
    if (data.contains("authorization_endpoint")) {
      std::string authorizationEndpoint = data["authorization_endpoint"].get<std::string>();
      if (data.contains("token_endpoint")) {
        std::string tokenEndpoint = data["token_endpoint"].get<std::string>();
        auto dashboardEndpoint = IAMHost + "/dashboard";
        auto scopes = "openid profile email offline_access";
        auto curl_device_data = "client_id=" + clientId + "&scope=" + scopes;
        curl = curl_easy_init();
        auto dashboardUrl = dashboardEndpoint + "?x509ClientAuth=true";
        if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, dashboardUrl.c_str());
          curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiesFile.c_str());
          curl_easy_setopt(curl, CURLOPT_SSLCERT, certFile.c_str());
          curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFile.c_str());
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed at dashboard authentication: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return "";
          }
          curl_easy_cleanup(curl);
        }
        auto authorizationUrl = authorizationEndpoint + "?response_type=code&client_id=" + clientId;
        curl = curl_easy_init();
        std::string readBufferCode;
        if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, authorizationUrl.c_str());
          curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
          curl_easy_setopt(curl, CURLOPT_SSLCERT, certFile.c_str());
          curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFile.c_str());
          curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
          curl_easy_setopt(curl, CURLOPT_HEADERDATA, &readBufferCode);
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed at authorization code request: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return "";
          }
          curl_easy_cleanup(curl);
        }
        auto code = get_code_from_headers(readBufferCode);
        if (!code.empty() && code[code.size() - 1] == '\r')
          code.erase(code.size() - 1);
        auto postdata = "code=" + code + "&grant_type=authorization_code&scope=" + scopes;
        curl = curl_easy_init();
        std::string readBufferToken;
        if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, tokenEndpoint.c_str());
          curl_easy_setopt(curl, CURLOPT_USERNAME, clientId.c_str());
          curl_easy_setopt(curl, CURLOPT_PASSWORD, clientSecret.c_str());
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBufferToken);
          res = curl_easy_perform(curl);
          if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_getinfo() failed at refresh token retrieval: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return "";
          }
          curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        if (nlohmann::json::accept(readBufferToken)) {
          nlohmann::json data = nlohmann::json::parse(readBufferToken);
          if (data.contains("refresh_token")) {
            Aws::String refreshToken = data["refresh_token"].get<std::string>();
            return refreshToken;
          }
        }
      }
    }
  }
  return "";
}
