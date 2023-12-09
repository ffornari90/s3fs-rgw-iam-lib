#include "main.hpp"
#include <string>
#include <cstdio>
#include <iostream>
#include <regex>
#include <cstdlib>
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

Aws::String getOIDCRefreshToken(const std::string &IAMHost, const std::string &clientId, const std::string &clientSecret,
                                const std::string &certFile, const std::string &keyFile, const std::string &cookiesFile,
                                const std::string &redirectUri, const std::string &scopes)
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

        std::string login_command = "OPENSSL_CONF=/etc/ssl "
        "phantomjs --ignore-ssl-errors=true --ssl-protocol=any "
        "--ssl-client-certificate-file=" + certFile +
        " --ssl-client-key-file=" + keyFile + " /usr/local/bin/login.js " +
        IAMHost + " " + cookiesFile;

        int login_commandResult = std::system(login_command.c_str());

        if (login_commandResult != 0) {
            fprintf(stderr, "Login command failed: %d\n", login_commandResult);
            return "";
        }

        std::string authorize_command = "OPENSSL_CONF=/etc/ssl "
        "phantomjs --ignore-ssl-errors=true --ssl-protocol=any "
        "--ssl-client-certificate-file=" + certFile +
        " --ssl-client-key-file=" + keyFile + " /usr/local/bin/authorize.js " +
        IAMHost + " " + clientId + " " + redirectUri + " " + cookiesFile;

        FILE* pipe = popen(authorize_command.c_str(), "r");

        if (!pipe) {
            perror("popen failed");
            return "";
        }

        char buffer[128];
        std::string authorize_commandOutput;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            authorize_commandOutput += buffer;
        }

        int authorize_commandResult = pclose(pipe);

        if (authorize_commandResult != 0) {
            fprintf(stderr, "Authorization command failed: %d\n", authorize_commandResult);
            return "";
        }

        std::regex codeRegex("code=([A-Za-z0-9_-]+)");
        std::smatch match;

        if (std::regex_search(authorize_commandOutput, match, codeRegex) && match.size() > 1) {
           std::string code = match.str(1);
           auto postdata = "code=" + code +
           "&grant_type=authorization_code&scope=" +
           scopes + "&redirect_uri=" + redirectUri;
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
        } else {
           std::cerr << "Failed to extract authorization code from the output." << std::endl;
           return "";
        }
      }
    }
  }
  return "";
}
