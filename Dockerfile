
#ARG DISTRO_VERSION =31
#fedora version 31 for docker image
FROM fedora:31
ARG NCPU=4

ENV DEBIAN_FRONTEND noninteractive
RUN dnf install -y git gcc-c++ gcc


LABEL Discription="This image is for testing of file discriptor as capebilities"

RUN dnf install -y cmake protobuf-compiler

RUN dnf install -y libseccomp libunwind libasan liblsan libtsan 
RUN dnf install -y libunwind  compiler-rt


#abseil installation

WORKDIR /var/tmp/build
RUN curl -sSL http://github.com/abseil/abseil-cpp/archive/20200225.2.tar.gz | \
tar -xzf - --strip-component=1 && \
cmake \
 -DCMAKE_BUILD_TYPE="Release" \
 -DBUILD_TESTING=OFF \
 -DBUILD_SHARED_LIBS=yes \
 -H. -Bcmake-out/abseil && \
cmake --build cmake-out/abseil --target install -- -j ${NCPU} && \
ldconfig && \
cd /var/tmp && rm -fr build

#googletest installation

WORKDIR /var/tmp/build
RUN curl -sSL https://github.com/google/googletest/archive/release-1.10.0.tar.gz | \
tar -xzf - --strip-component=1 && \
cmake \
 -DCMAKE_BUILD_TYPE="Release" \
 -DBUILD_SHARED_LIBS=yes \
 -H. -Bcmake-out/googletest && \
cmake --build cmake-out/googletest --target install -- -j ${NCPU} && \
ldconfig && \
cd /var/tmp && rm -fr build

#google log installation

RUN dnf install -y glog

RUN dnf update -y
RUN dnf upgrade -y
LABEL com ="completed installation" 
