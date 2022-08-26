# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# Docker file for Arch Linux contianer for building and create packages
###############################################################################
FROM archlinux
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
ARG UID
ARG USER
RUN patched_glibc=glibc-linux4-2.35-2-x86_64.pkg.tar.zst &&\
    curl -LO "https://repo.archlinuxcn.org/x86_64/$patched_glibc" &&\
    bsdtar -C / -xvf "$patched_glibc" &&\
    pacman -Syu --noconfirm --needed bash rustup gcc fakeroot pkgconf libtool\
    which make git sudo doxygen graphviz perl ruby swig python3\
    lua lua51 lua52 lua53 vim tcl json-c m4 &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
USER $USER
ARG SRC
COPY --chown=builder $SRC /home/$USER/libptpmgmt
