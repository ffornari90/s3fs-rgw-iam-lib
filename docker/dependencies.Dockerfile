FROM ubuntu:22.04
RUN apt-get update \
 && apt-get install -y git cmake curl wget uuid-dev zlib1g-dev libpulse-dev \
    libcurl4-openssl-dev autotools-dev automake build-essential \
    libxml2-dev pkg-config libssl-dev libfuse-dev fuse && \
    apt-get clean
RUN curl "https://ssl-tools.net/certificates/c2826e266d7405d34ef89762636ae4b36e86cb5e.pem" \
    -o /usr/local/share/ca-certificates/geant-ov-rsa-ca.crt && \
    wget -q -O - "https://dist.eugridpma.info/distribution/igtf/current/GPG-KEY-EUGridPMA-RPM-3" | apt-key add - && \
    echo "deb http://repository.egi.eu/sw/production/cas/1/current egi-igtf core" > /etc/apt/sources.list.d/ca-repo.list && \
    apt-get update && apt-get install -y ca-policy-egi-core && apt-get clean && \
    for f in `find /etc/grid-security/certificates -type f -name '*.pem'`; \
    do filename="${f##*/}"; cp $f /usr/local/share/ca-certificates/"${filename%.*}.crt"; done && \
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
