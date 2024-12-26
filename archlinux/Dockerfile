# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Arch Linux container for building and create packages
# Search packages at: https://archlinux.org/packages/
# pacman: https://wiki.archlinux.org/title/pacman
###############################################################################
FROM archlinux
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
LABEL org.opencontainers.image.description "Arch Linux build for libptpmgmt"
ARG UID
ARG USER
RUN pacman-key --init && pacman-key --populate &&\
    pacman -Syu --noconfirm --needed archlinux-keyring &&\
    pacman -Syu --noconfirm --needed bash rustup gcc fakeroot pkgconf libtool\
    autoconf which make git sudo doxygen graphviz perl ruby swig python3\
    lua lua51 lua52 lua53 vim tcl json-c php m4 gtest texlive-core go jq\
    texlive-fontutils astyle lua-posix lua51-posix lua52-posix lua53-posix\
    clang openssl gnutls nettle gdb debugedit base-devel chrpath patchelf\
    discount ruby-test-unit &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers &&\
    mkdir -p /t0 && cd /t0 && git clone https://github.com/linux-rt/librtpi &&\
    cd librtpi && autoreconf -i && ./configure --prefix=/usr && make install &&\
    cd /t0 && git clone https://gitlab.com/chrony/libchrony &&\
    cd libchrony && make install prefix=/usr && cd && rm -Rf /t0
ARG COOKIE
RUN pacman -Syu --noconfirm --needed
ARG SRC
ARG DST
COPY --chown=builder $SRC $DST
