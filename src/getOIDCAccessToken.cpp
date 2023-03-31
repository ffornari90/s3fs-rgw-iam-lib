#include "main.hpp"
#include <stdio.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Aws::String getOIDCAccessToken(const std::string &IAMHost, const std::string &refreshToken,
                               const std::string &clientId, const std::string &clientSecret)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  auto curl_config_url = IAMHost + "/.well-known/openid-configuration";
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, curl_config_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  if (nlohmann::json::accept(readBuffer)) {
    nlohmann::json data = nlohmann::json::parse(readBuffer);
    if (data.contains("token_endpoint")) {
      std::string tokenEndpoint = data["token_endpoint"].get<std::string>();
      auto curl_token_data = "grant_type=refersh_token&refresh_token=" + refreshToken;
      if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, tokenEndpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, clientId.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, clientSecret.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curl_token_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
        curl_easy_cleanup(curl);
      }
      curl_global_cleanup();
      if (nlohmann::json::accept(readBuffer)) {
        nlohmann::json data = nlohmann::json::parse(readBuffer);
        if (data.contains("access_token")) {
          Aws::String accessToken = data["access_token"].get<std::string>();
          return accessToken;
        }
      }
    }
  }
  return "";
}
