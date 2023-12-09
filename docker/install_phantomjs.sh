#!/bin/bash -x
# This script installs PhantomJS in your Debian/Ubuntu or RedHat/CentOS System
#
# This script must be run as root:
# sudo sh install_phantomjs.sh
#

if [[ $EUID -ne 0 ]]; then
        echo "This script must be run as root" 1>&2
        exit 1
fi

# Check the system to determine package manager and commands
if [[ -f /etc/debian_version ]]; then
    PM="apt-get"
    INSTALL="install"
elif [[ -f /etc/redhat-release ]]; then
    PM="yum"
    INSTALL="install"
fi

if [ $PM == "apt-get" ]; then
    $PM update
    $PM -y $INSTALL build-essential chrpath libssl-dev libxft-dev libfreetype6 libfreetype6-dev libfontconfig1 libfontconfig1-dev wget curl
elif [ $PM == "yum" ]; then
    $PM -y update
    $PM -y $INSTALL fontconfig freetype freetype-devel fontconfig-devel wget bzip2
fi

PHANTOM_JS_LATEST=$(curl -s https://bitbucket.org/ariya/phantomjs/downloads/ | grep -i -e zip -e bz2 | grep -vi beta | grep -i linux-x86_64 | grep -v symbols | cut -d '>' -f 2 | cut -d '<' -f 1 | head -n 1)
PHANTOM_VERSION=${PHANTOM_JS_LATEST%-*-*.*.*}
ARCH=$(uname -m)

if ! [ $ARCH = "x86_64" ]; then
        ARCH="i686"
fi

PHANTOM_JS="$PHANTOM_VERSION-linux-$ARCH"

cd ~
wget https://bitbucket.org/ariya/phantomjs/downloads/$PHANTOM_JS.tar.bz2
tar xvjf $PHANTOM_JS.tar.bz2

mv $PHANTOM_JS /usr/local/share
ln -sf /usr/local/share/$PHANTOM_JS/bin/phantomjs /usr/local/bin
