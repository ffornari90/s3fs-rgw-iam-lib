#pragma once
#include <iostream>
#include <aws/s3/S3Client.h>
#include <aws/sts/STSClient.h>
#include <aws/sts/model/AssumeRoleRequest.h>
#include <aws/sts/model/AssumeRoleWithWebIdentityRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/ListBucketsResult.h>
#include <aws/core/Aws.h>

namespace AwsDoc {
    namespace STS {
        bool assumeRoleWithWebIdentity(const Aws::String &roleArn,
                        const Aws::String &roleSessionName,
                        const Aws::String &refreshToken,
                        const Aws::String &webIdentityToken,
                        Aws::Auth::AWSCredentials &credentials,
                        const Aws::Client::ClientConfiguration &clientConfig);
    }
}

void configureClient(Aws::Client::ClientConfiguration &clientConfig,
                     Aws::String &endpointOverride,
                     Aws::String &region);

Aws::String getOIDCRefreshToken(const std::string &IAMHost, const std::string &clientId, const std::string &clientSecret,
                                const std::string &certFile, const std::string &keyFile, const std::string &cookiesFile);
Aws::String getOIDCAccessToken(const std::string &IAMHost, const std::string &RGWHost,
                               const std::string &refreshToken,
                               const std::string &clientId, const std::string &clientSecret);

extern "C" {
extern const char* VersionS3fsCredential(bool detail);
extern bool InitS3fsCredential(const char* popts, char** pperrstr);
extern bool FreeS3fsCredential(char** pperrstr);
extern bool UpdateS3fsCredential(char** ppaccess_key_id, char** ppserect_access_key,
                                 char** ppaccess_token, long long* ptoken_expire,
                                 char** pperrstr);
}
