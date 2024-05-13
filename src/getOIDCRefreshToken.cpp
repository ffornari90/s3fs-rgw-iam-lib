#include "main.hpp"
#include <string>
#include <stdio.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Aws::String getOIDCRefreshToken(const std::string &IAMHost, const std::string &clientId, const std::string &clientSecret)
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
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed at openid configuration discovery: %s\n",
              curl_easy_strerror(res));
    curl_easy_cleanup(curl);
  }
  fprintf(stderr, "curl_response: %s\n", readBufferDiscovery);
  if (nlohmann::json::accept(readBufferDiscovery)) {
    nlohmann::json data = nlohmann::json::parse(readBufferDiscovery);
    if (data.contains("device_authorization_endpoint")) {
      std::string deviceEndpoint = data["device_authorization_endpoint"].get<std::string>();
      if (data.contains("token_endpoint")) {
        std::string tokenEndpoint = data["token_endpoint"].get<std::string>();
        auto scopes = "openid profile email offline_access";
        auto curl_device_data = "client_id=" + clientId + "&scope=" + scopes;
        std::string readBufferDevice;
        curl = curl_easy_init();
        if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, deviceEndpoint.c_str());
          curl_easy_setopt(curl, CURLOPT_USERNAME, clientId.c_str());
          curl_easy_setopt(curl, CURLOPT_PASSWORD, clientSecret.c_str());
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curl_device_data.c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBufferDevice);
          curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
          res = curl_easy_perform(curl);
          if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed at device code initialization: %s\n",
                    curl_easy_strerror(res));
          curl_easy_cleanup(curl);
        }
        if (nlohmann::json::accept(readBufferDevice)) {
          nlohmann::json data = nlohmann::json::parse(readBufferDevice);
          if (data.contains("device_code")) {
            std::string deviceCode = data["device_code"].get<std::string>();
            if (data.contains("user_code")) {
              std::string userCode = data["user_code"].get<std::string>();
              if (data.contains("verification_uri")) {
                std::string verificationUri = data["verification_uri"].get<std::string>();
                if (data.contains("expires_in")) {
                  int expires_in = data["expires_in"].get<int>();
                  std::string expiresIn = std::to_string(expires_in);
                  printf("Please open the following URL in the browser:\n\n");
                  printf("%s\n\n", verificationUri.c_str());
                  printf("and, after having been authenticated, enter the following code when requested:\n\n");
                  printf("%s\n\n", userCode.c_str());
                  printf("Note that the code above expires in %s seconds...\n", expiresIn.c_str());
                  char a;
                  while (true) {
                    printf("\nProceed? [Y/N] (CTRL-c to abort)\n");
                    scanf(" %c", &a);
                    if (a == 'y' || a == 'Y') {
                      break;
                    } else if (a == 'n' || a == 'N') {
                      return "";
                    }
                  }
                  auto curl_token_data = "grant_type=urn:ietf:params:oauth:grant-type:device_code&device_code=" + deviceCode;
                  std::string readBufferRefresh;
                  curl = curl_easy_init();
                  if(curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, tokenEndpoint.c_str());
                    curl_easy_setopt(curl, CURLOPT_USERNAME, clientId.c_str());
                    curl_easy_setopt(curl, CURLOPT_PASSWORD, clientSecret.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curl_token_data.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBufferRefresh);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                    res = curl_easy_perform(curl);
                    if(res != CURLE_OK)
                      fprintf(stderr, "curl_easy_perform() failed at refresh token retrieval: %s\n",
                              curl_easy_strerror(res));
                    curl_easy_cleanup(curl);
                  }
                  curl_global_cleanup();
                  if (nlohmann::json::accept(readBufferRefresh)) {
                    nlohmann::json data = nlohmann::json::parse(readBufferRefresh);
                    if (data.contains("refresh_token")) {
                      Aws::String refreshToken = data["refresh_token"].get<std::string>();
                      return refreshToken;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return "";
}
