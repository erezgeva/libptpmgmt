# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Debian contianers for building and create deb packages
# Search packages at: https://packages.debian.org
###############################################################################
ARG DIST
FROM debian:${DIST}
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
LABEL org.opencontainers.image.description "Debian build for libptpmgmt"
ARG UID
ARG USER
ARG DIST
ARG ARCHS
ARG SRC_CFG
ARG DPKGS
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update;\
    apt-get install -y --no-install-recommends gnupg ca-certificates\
    apt-utils;for a in $ARCHS;do dpkg --add-architecture $a;\
    done;printf "$SRC_CFG" > /etc/apt/sources.list; apt-get update;\
    apt-get install -y --no-install-recommends debhelper coreutils sudo\
    dh-lua dh-make-perl dh-exec git procps dh-php build-essential ncurses-bin\
    dlocate dialog lsb-release quilt apt-file libtool libtool-bin swig golang\
    install-info doxygen graphviz qemu-user-static distro-info\
    tree lua5.1 lua5.2 lua5.3 dh-python python3-dev php-dev m4 perl ruby\
    astyle cppcheck universal-ctags libtest-class-perl ruby-test-unit lua-unit\
    ruby-test-unit-context ruby-test-unit-notify libfile-touch-perl phpunit\
    python3-demjson lua5.4 valgrind dh-golang reuse vim-gtk3 libgcrypt20-dev\
    texlive-font-utils libcriterion-dev jq clang gdb abi-dumper\
    abi-compliance-checker $DPKGS;\
    apt-get upgrade -y --no-install-recommends;\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php/*/cli/php.ini;\
    useradd $USER -u $UID -m -G sudo;\
    sed -i '/%sudo/ s/ ALL$/ NOPASSWD:ALL/' /etc/sudoers;\
    if test "$DIST" = "bookworm";then mkdir -p /tmp/1;\
    cd /tmp/1;git clone https://salsa.debian.org/fmoessbauer/librtpi.git;\
    cd librtpi;dpkg-buildpackage -b --no-sign;for a in $ARCHS;\
    do DEB_BUILD_OPTIONS=nocheck dpkg-buildpackage -B --no-sign -a$a;done;\
    cd ..;dpkg -i librtpi-dev_*.deb librtpi1_*.deb;cd ..;rm -Rf 1;fi
ARG COOKIE
RUN apt-get update; apt-get upgrade -y --no-install-recommends
ARG SRC
ARG DST
COPY --chown=builder $SRC $DST
