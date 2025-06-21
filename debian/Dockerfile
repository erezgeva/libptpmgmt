# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Debian containers for building and create deb packages
# Search packages at: https://packages.debian.org
###############################################################################
ARG DIST
FROM debian:${DIST}
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
LABEL org.opencontainers.image.description "Debian build for libptpmgmt"
ARG UID USER DIST ARCH ARCHS DPKGS REPO DEBIAN_FRONTEND=noninteractive
RUN if [ -f /etc/apt/sources.list.d/debian.sources ];then\
    sed -i "s|http://deb.debian.org/debian|$REPO|"\
    /etc/apt/sources.list.d/debian.sources;fi &&\
    if [ -f /etc/apt/sources.list ];then\
    sed -i "s|http://deb.debian.org/debian|$REPO|;/^#/ d"\
    /etc/apt/sources.list;fi &&\
    apt-get update && apt-get install -y --no-install-recommends --no-remove\
    gnupg ca-certificates apt-utils &&\
    for a in $ARCHS;do dpkg --add-architecture $a;done &&\
    apt-get update && apt-get install -y --no-install-recommends --no-remove\
    debhelper coreutils sudo dh-lua dh-make-perl dh-exec git procps dh-php\
    build-essential ncurses-bin dlocate dialog lsb-release quilt apt-file\
    libtool libtool-bin swig golang install-info doxygen graphviz\
    qemu-user-static distro-info chrpath patchelf tree tclx8.4\
    lua5.1 lua5.2 lua5.3 dh-python python3-dev php-dev m4 perl ruby\
    astyle cppcheck universal-ctags libtest-class-perl ruby-test-unit lua-unit\
    ruby-test-unit-context ruby-test-unit-notify libfile-touch-perl phpunit\
    python3-demjson lua5.4 valgrind dh-golang reuse vim-gtk3 libgcrypt20-dev\
    texlive-font-utils libcriterion-dev jq clang gdb abi-dumper gawk wget\
    abi-compliance-checker discount chrony cmark-gfm tcllib $DPKGS &&\
    apt-get upgrade -y --no-install-recommends &&\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php/*/cli/php.ini &&\
    useradd $USER -u $UID -m -G sudo &&\
    sed -i '/%sudo/ s/ ALL$/ NOPASSWD:ALL/' /etc/sudoers &&\
    for a in $ARCHS;do list=`dpkg --get-selections | grep "linux-gnu:$a" |\
    sed "s/\s*install/ /;s/:$a/:$ARCH/" | tr -d $'\r\n'` &&\
    if [ -n "$list" ];then apt-get install -y --no-install-recommends $list\
    ;fi;done
COPY debian/pkgs /tmp/pkgs
RUN dpkg -i /tmp/pkgs/$DIST/*.deb && rm -R /tmp/pkgs
ARG COOKIE
RUN apt-get update && apt-get upgrade -y --no-install-recommends
ARG SRC DST
COPY --chown=builder $SRC $DST
