#!/usr/bin/env bash

function update() {
    repo=`basename $1 .git`
    echo "updating $repo..."
    if [ -d $repo ]; then
        (cd $repo && git pull)
    else
        git clone $1
    fi
    [ -z "$2" ] || (cd $repo && git checkout $2)
}

function update_SAI() {
    repo="SAI"
    echo "updating $repo..."
    if [ -d $repo ]; then
        (cd $repo && git pull) 
    else
        git clone https://github.com/opencomputeproject/SAI.git $repo
    fi
    (cd $repo && git checkout v0.9.3.0)
    (cd $repo && mkdir -p lib)
    (cd $repo && ln -s sai/inc inc) 
}

function update_branch() {
    branch="$1"
    if [ "$(git symbolic-ref -q --short HEAD)" != "OpenNSL_6.3" ]; then
        git checkout -tb "$branch" "origin/$branch"
    fi
}

function build() {
    (
        echo "building $1..."
        cd $1
        if [ -e ./CMakeLists.txt ]; then
            mkdir -p build
            cd build
            echo cmake .. $CMAKEFLAGS
            cmake .. $CMAKEFLAGS
            make
        else
            if [ ! -e ./configure ]; then
                autoreconf --install
            fi
            ./configure
            make -j8
        fi
    )
}

echo "installing packages"
sudo apt-get install -yq autoconf automake libdouble-conversion-dev \
    libssl-dev make zip git autoconf libtool g++ libboost-all-dev \
    libevent-dev flex bison libgoogle-glog-dev scons libkrb5-dev \
    libsnappy-dev libsasl2-dev libnuma-dev libi2c-dev libcurl4-nss-dev \
    libusb-1.0-0-dev libpcap-dev libdb5.3-dev cmake

echo "creating external..."
mkdir -p external
(
    cd external
    # We hard code OpenNSL to OpenNSL-6.4.6.6 release, later releases seem to
    # SIGSEV in opennsl_pkt_alloc()
    update https://github.com/Broadcom-Switch/OpenNSL.git 8e0b499f02dcef751a3703c9a18600901374b28a
    update \
        git://git.kernel.org/pub/scm/linux/kernel/git/shemminger/iproute2.git v3.19.0
    update https://github.com/facebook/folly.git 08dba5714790020d2fa677e34e624eb4f34a20ca
    update https://github.com/facebook/wangle.git d67b7632be2923de3695201c7ac361f50646bbbf
    update https://github.com/facebook/fbthrift.git 1b2b03a472c41915a8c481a06edc630674377e77
    update_SAI
    build iproute2
    build folly/folly 
    export CMAKEFLAGS=-D"FOLLY_INCLUDE_DIR=`pwd`/folly"\ -D"FOLLY_LIBRARY=`pwd`/folly/folly/.libs/libfolly.a"\ -D"BUILD_TESTS=OFF"
    build wangle/wangle
    export CPPFLAGS=" -I`pwd`/folly -I`pwd`/wangle" LDFLAGS="-L`pwd`/folly/folly/.libs/ -L`pwd`/wangle/wangle/build/lib"
    build fbthrift/thrift
)
