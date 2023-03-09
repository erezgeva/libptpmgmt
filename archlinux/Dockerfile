# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Docker file for Arch Linux contianer for building and create packages
# Search packages at: https://archlinux.org/packages/
###############################################################################
FROM archlinux
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
ARG UID
ARG USER
RUN pacman -Syu --noconfirm --needed bash rustup gcc fakeroot pkgconf libtool\
    autoconf which make git sudo doxygen graphviz perl ruby swig python3\
    lua lua51 lua52 lua53 vim tcl json-c php m4 gtest texlive-core &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
ARG COOKIE
RUN pacman -Syu --noconfirm --needed
USER $USER
ARG SRC
COPY --chown=builder $SRC /home/$USER/libptpmgmt
