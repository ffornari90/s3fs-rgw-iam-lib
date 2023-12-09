FROM alpine:latest
RUN apk update && apk add git cmake curl wget util-linux-dev zlib-dev pulseaudio-dev \
    curl-dev autoconf automake build-base \
    libxml2-dev pkgconfig openssl-libs-static fuse fuse-dev fio && \
    rm -rf /var/cache/apk/*
RUN wget -O /usr/local/share/ca-certificates/geant-ov-rsa-ca.crt 'https://crt.sh/?d=2475254782' && \
    wget 'http://repository.egi.eu/sw/production/cas/1/current/tgz/' && mkdir tgz && mkdir certificates && \
    for tgz in $(cat index.html | awk -F'"' '{print $2}' | grep tar.gz); do wget 'http://repository.egi.eu/sw/production/cas/1/current/tgz/'$tgz -O tgz/$tgz; done && \
    for tgz in $(ls tgz/); do tar xzf tgz/$tgz --strip-components=1 -C certificates/; done && \
    for f in $(find certificates/ -type f -name '*.pem'); do filename="${f##*/}"; cp $f /usr/local/share/ca-certificates/"${filename%.*}.crt"; done && \
    update-ca-certificates
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp && \
    mkdir sdk_build && cd sdk_build && \
    cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$PWD/../aws-sdk-cpp \
    -DBUILD_ONLY="core;identity-management;iam;s3;sts" \
    -DAUTORUN_UNIT_TESTS=OFF && make && make install && cd .. && \
    git clone https://github.com/nlohmann/json.git && \
    cd json && cmake -S . -B build && cmake --build build && \
    cd build && make install && cd ../.. && \
    rm -rf aws-sdk-cpp sdk_build json
