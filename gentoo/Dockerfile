# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Docker file for Gentoo container for building and installing
# Search packages at: https://portagefilelist.de/
###############################################################################
FROM gentoo/stage3
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
LABEL org.opencontainers.image.description "Gentoo build for libptpmgmt"
ARG UID
ARG USER
COPY gentoo/pkgs_use /etc/portage/package.use/pkgs
RUN emerge-webrsync && emerge -tv app-portage/portage-utils app-shells/bash\
    app-admin/sudo doxygen app-editors/vim sys-apps/fakeroot dev-lua/luaposix\
    sys-apps/which sys-devel/gcc libtool make dev-lang/go sys-devel/m4\
    dev-util/pkgconf dev-vcs/git dev-lang/swig dev-lang/perl dev-lang/ruby\
    dev-lang/python dev-lang/lua dev-lang/php dev-lang/tcl app-misc/jq\
    dev-libs/criterion media-gfx/graphviz app-text/texlive-core\
    dev-texlive/texlive-fontutils dev-cpp/gtest dev-util/astyle\
    llvm-core/clang dev-debug/gdb app-admin/chrpath dev-util/patchelf\
    app-text/discount &&\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php/*/php.ini &&\
    useradd $USER -u $UID -m -G users,wheel,portage &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers &&\
    mkdir -p /t0 && cd /t0 && git clone https://github.com/linux-rt/librtpi &&\
    cd librtpi && autoreconf -i &&\
    ./configure --prefix=/usr --includedir=/usr/include --libdir=/usr/lib64 &&\
    make install &&\
    cd /t0 && git clone https://gitlab.com/chrony/libchrony &&\
    cd libchrony && make install includedir=/usr/include libdir=/usr/lib64 &&\
    cd && rm -Rf /t0
ARG COOKIE
RUN emerge --oneshot -tv sys-apps/portage
ARG SRC
ARG DST
COPY --chown=builder $SRC $DST
