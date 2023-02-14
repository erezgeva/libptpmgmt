# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Fedora contianer for building and create rpm packages
###############################################################################

ARG DIST
FROM debian:${DIST}
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
ARG UID
ARG USER
ARG ARCHS
ARG SRC_CFG
ARG DPKGS
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update;\
    apt-get install -y --no-install-recommends gnupg apt-transport-https\
    ca-certificates apt-utils;for a in $ARCHS;do dpkg --add-architecture $a;\
    done;printf "$SRC_CFG" > /etc/apt/sources.list; apt-get update;\
    apt-get install -y --no-install-recommends debhelper coreutils sudo\
    dh-lua dh-make-perl dh-exec git procps dh-php build-essential ncurses-bin\
    dlocate dialog lsb-release quilt apt-file libtool libtool-bin swig\
    install-info libjson-c-dev doxygen graphviz qemu-user-static distro-info\
    tree lua5.1 lua5.2 lua5.3 dh-python python3-dev php-dev m4 perl ruby\
    astyle cppcheck exuberant-ctags libtest-class-perl ruby-test-unit lua-unit\
    ruby-test-unit-context ruby-test-unit-notify libfile-touch-perl phpunit\
    $DPKGS;for a in $ARCHS;do all='';for p in libperl-dev;do all="$all $p:$a";\
    done; apt-get install -y --no-install-recommends $all;done;\
    apt-get install -y --no-install-recommends libperl-dev;\
    apt-get upgrade -y --no-install-recommends;\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php/*/cli/php.ini;\
    useradd $USER -u $UID -m -G sudo;\
    sed -i '/%sudo/ s/ ALL$/ NOPASSWD:ALL/' /etc/sudoers
USER $USER
ARG SRC
COPY --chown=builder $SRC /home/$USER/libptpmgmt
