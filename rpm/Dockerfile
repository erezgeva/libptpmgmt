# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Fedora container for building and create rpm packages
# Search packages at: https://packages.fedoraproject.org/
###############################################################################
FROM fedora
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
LABEL org.opencontainers.image.description "RPM build for libptpmgmt"
ARG UID USER
RUN dnf install -y gcc gcc-c++ libtool libtool-ltdl make cmake git pkgconfig\
    sudo automake autoconf yum-utils rpm-build dnf-utils fedora-packager\
    rpmdevtools doxygen graphviz perl php-devel ruby ruby-devel swig astyle\
    lua lua-devel python3-devel php python3 which gtest-devel gtest golang\
    perl-devel perl-ExtUtils-Embed lua-posix tcl tcl-devel clang openssl-libs\
    openssl-devel libgcrypt-devel gnutls-devel nettle-devel perl-Test-Class\
    libfastjson libfastjson-devel json-c-devel m4 rubygem-test-unit gdb tclx\
    texlive-epstopdf ghostscript php-phpunit-PHPUnit perl-File-Touch jq\
    chrpath patchelf discount gmock-devel cppcheck chrony quilt cmark tcllib\
    && dnf clean all &&\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php.ini &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
COPY rpm/pkgs /tmp
RUN dnf install -y /tmp/*.rpm && rm /tmp/*.rpm
ARG COOKIE
RUN dnf upgrade -y
ARG SRC DST
COPY --chown=builder $SRC $DST
