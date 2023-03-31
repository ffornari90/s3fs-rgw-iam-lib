#include "main.hpp"
#include <time.h>

int main(int argc, char** argv)
{
	char* perrstr = NULL;

	std::cout << "[awscred_test] Start test for s3fsrgwsts.so" << std::endl;
	std::cout << std::endl;

	//
	// Test : InitS3fsCredential
	//
	std::cout << "  [Function] InitS3fsCredential" << std::endl;
	if(!InitS3fsCredential("Debug", &perrstr)){
		std::cerr << "     [ERROR] Could not initialize s3fsrgwsts.so : " << (perrstr ? perrstr : "unknown") << std::endl;
		if(perrstr){
			free(perrstr);
		}
		exit(EXIT_FAILURE);
	}
	std::cout << "     [Succeed]" << std::endl;
	std::cout << std::endl;

	//
	// Test : VersionS3fsCredential
	//
	std::cout << "  [Function] VersionS3fsCredential - short" << std::endl;
	const char* pVersion = VersionS3fsCredential(false);
	if(!pVersion){
		std::cerr << "     [ERROR] Version string(not detail) is NULL." << std::endl;
		FreeS3fsCredential(&perrstr);
		if(perrstr){
			free(perrstr);
		}
		exit(EXIT_FAILURE);
	}
	std::cout << "     [Succeed] " << pVersion << std::endl;
	std::cout << std::endl;

	std::cout << "  [Function] VersionS3fsCredential - detail" << std::endl;
	pVersion = VersionS3fsCredential(true);
	if(!pVersion){
		std::cerr << "     [ERROR] Version string(detail) is NULL." << std::endl;
		FreeS3fsCredential(&perrstr);
		if(perrstr){
			free(perrstr);
		}
		exit(EXIT_FAILURE);
	}
	std::cout << "     [Succeed] " << std::endl;
	std::cout << pVersion;
	std::cout << std::endl;

	//
	// Test : UpdateS3fsCredential
	//
	char*		paccess_key_id		= NULL;
	char*		pserect_access_key	= NULL;
	char*		paccess_token		= NULL;
	long long	token_expire		= 0;

	std::cout << "  [Function] UpdateS3fsCredential" << std::endl;
	if(!UpdateS3fsCredential(&paccess_key_id, &pserect_access_key, &paccess_token, &token_expire, &perrstr)){
		std::cerr << "     [ERROR] Could not get Credential for AWS : " << (perrstr ? perrstr : "unknown") << std::endl;
		if(perrstr){
			free(perrstr);
		}
		FreeS3fsCredential(&perrstr);
		if(perrstr){
			free(perrstr);
		}
		exit(EXIT_FAILURE);
	}
	std::cout << "     [Succeed] Credential = {" << std::endl;
	std::cout << "                 AWS Access Key Id    = " << paccess_key_id << std::endl;
	std::cout << "                 AWS Secret Key       = " << pserect_access_key << std::endl;
	std::cout << "                 AWS Session Token    = " << paccess_token << std::endl;
	std::cout << "                 Expiration(unixtime) = " << token_expire	<< std::endl;
	std::cout << "               }"	<< std::endl;
	std::cout << std::endl;

	//
	// Test : FreeS3fsCredential
	//
	std::cout << "  [Function] FreeS3fsCredential" << std::endl;
	if(!FreeS3fsCredential(&perrstr)){
		std::cerr << "     [ERROR] Could not uninitialize s3fsrgwsts.so : " << (perrstr ? perrstr : "unknown") << std::endl;
		if(perrstr){
			free(perrstr);
		}
		exit(EXIT_FAILURE);
	}
	std::cout << "     [Succeed]" << std::endl;
	std::cout << std::endl;

	exit(EXIT_SUCCESS);
}
