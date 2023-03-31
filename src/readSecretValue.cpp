#include "main.hpp"
#include <fstream>

bool AwsDoc::STS::assumeRoleWithWebIdentity(const Aws::String &roleArn,
                             const Aws::String &roleSessionName,
                             const Aws::String &refreshToken,
                             const Aws::String &webIdentityToken,
                             Aws::Auth::AWSCredentials &credentials,
                             const Aws::Client::ClientConfiguration &clientConfig) {
    Aws::STS::STSClient sts(clientConfig);
    Aws::STS::Model::AssumeRoleWithWebIdentityRequest sts_req;

    sts_req.SetRoleArn(roleArn);
    sts_req.SetRoleSessionName(roleSessionName);
    sts_req.SetWebIdentityToken(webIdentityToken);

    const Aws::STS::Model::AssumeRoleWithWebIdentityOutcome outcome = sts.AssumeRoleWithWebIdentity(sts_req);

    if (!outcome.IsSuccess()) {
        std::cerr << "Error assuming IAM role. " <<
                  outcome.GetError().GetMessage() << std::endl;
    }
    else {
        const Aws::STS::Model::AssumeRoleWithWebIdentityResult result = outcome.GetResult();
        const Aws::STS::Model::Credentials &temp_credentials = result.GetCredentials();

        auto home = std::getenv("HOME") ? std::getenv("HOME") : "/tmp";
        std::string s3fs_credfile = "/.aws/credentials";
        std::ofstream ofs(home + s3fs_credfile, std::ofstream::trunc);
        ofs << "[default]\n";
        ofs << "aws_access_key_id = " + temp_credentials.GetAccessKeyId() + "\n";
        ofs << "aws_secret_access_key = " + temp_credentials.GetSecretAccessKey() + "\n";
        ofs << "aws_session_token = " + temp_credentials.GetSessionToken() + "\n";
        ofs << "aws_refresh_token = " + refreshToken + "\n";
        ofs.close();

        credentials.SetAWSAccessKeyId(temp_credentials.GetAccessKeyId());
        credentials.SetAWSSecretKey(temp_credentials.GetSecretAccessKey());
        credentials.SetSessionToken(temp_credentials.GetSessionToken());
    }

    return outcome.IsSuccess();
}
