# s3fs-rgw-iam-lib

Authentication module using [AWS C++ SDK](https://github.com/aws/aws-sdk-cpp) for [s3fs-fuse](https://github.com/s3fs-fuse/s3fs-fuse/) to locally mount [Ceph RADOS Gateway](https://docs.ceph.com/en/latest/radosgw/index.html) buckets.

## Overview
`s3fs-rgw-iam-lib` is a shared library that performs credential processing of s3fs-fuse.
This shared library can be specified with the option (`credlib` and `credlib_opts`) of s3fs-fuse and works by replacing the built-in credential processing of s3fs-fuse.
It makes use of Curl C API in order to get an OIDC access token from Indigo-IAM and get the client authenticated and authorized; then, using AWS C++ SDK, it obtains S3 temporary credentials from Ceph RADOS Gateway to mount buckets with s3fs-fuse. 

## Usage
You can easily build and use `s3fs-rgw-iam-lib` by following the steps below.

### Build

#### Build and Install nlohmann/json on Ubuntu22.04
```
$ git clone https://github.com/nlohmann/json.git
$ cd json
$ cmake -S . -B build
$ cmake --build build
$ cd build
$ sudo make install
```

#### Build and Install AWS-SDK-CPP on Ubuntu22.04
```
$ sudo apt-get install libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev
$ git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
$ mkdir sdk_build
$ cd sdk_build
$ cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$PWD/../aws-sdk-cpp -DBUILD_ONLY="core;identity-management;iam;s3;sts" -DAUTORUN_UNIT_TESTS=OFF 
$ make
$ sudo make install
```

#### Build s3fs-fuse and s3fs-rgw-iam-lib
```
$ sudo apt-get install -y autotools-dev automake build-essential libxml2-dev pkg-config libssl-dev libfuse-dev fuse
$ git clone git@baltig.infn.it:fornari/s3fs-rgw-iam-lib.git
$ cd s3fs-rgw-iam-lib
$ cmake -S . -B build
$ cd build
$ sudo make install
```
After that, you can find `librgw-sts.so` in `build` sub directory.

### Run s3fs-fuse
You can set a profile to be sourced before s3fs-fuse execution in order to configure your environment.
For example:
```
$ cat rgw-sts-profile
AWS_EC2_METADATA_DISABLED=true
ROLE_ARN=arn:aws:iam:::role/<role_name>
ROLE_NAME=<role_session_name>
IAM_HOST=https://iam.example.com
CLIENT_ID=<client_id>
CLIENT_SECRET=<client_secret>
ENDPOINT_URL=https://radosgw.example.com
REGION_NAME=<region_name>
BUCKET_NAME=<bucket_name>
```
Then, you can run s3fs-fuse in the following way:
```
$ export $(grep -v '^#' rgw-sts-profile | xargs -d '\n')
$ s3fs <bucket> <mountpoint> <options...> -o credlib=librgw-sts.so -o credlib_opts=Off
```
To specify this `s3fs-rgw-iam-lib` for s3fs, use the following options:

#### credlib
An option to specify the `s3fs-rgw-iam-lib` library.
You can specify only the library name or the path to the library file.
The s3fs use `dlopen` to search for the specified `s3fs-rgw-iam-lib` library and load it.

Example:
```
-o credlib=librgw-sts.so
```

#### credlib_opts
Specifies the options provided by `s3fs-rgw-iam-lib`.
If you specify `s3fs-rgw-iam-lib`, you can specify the output level of the debug message shown below for this option:
- Off
- Fatal
- Error
- Warn
- Info
- Debug
- Trace

Example:
```
-o credlib_opts=Info
```
