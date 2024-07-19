# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Arch Linux contianer for building and create packages
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
    clang openssl gnutls nettle gdb debugedit base-devel &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers;\
    mkdir -p /tmp/1;cd /tmp/1;git clone https://github.com/linux-rt/librtpi;\
    cd librtpi;autoreconf --install;./configure --prefix=/usr;make install;\
    cd /tmp;rm -Rf 1
ARG COOKIE
RUN pacman -Syu --noconfirm --needed
ARG SRC
ARG DST
COPY --chown=builder $SRC $DST
