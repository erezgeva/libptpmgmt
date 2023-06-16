# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Docker file for Gentoo contianer for building and create packages
# Search packages at: https://portagefilelist.de/
###############################################################################
FROM gentoo/stage3
MAINTAINER "Erez Geva" <ErezGeva2@gmail.com>
ARG UID
ARG USER
RUN emerge-webrsync && emerge -tv app-portage/portage-utils app-shells/bash\
    app-admin/sudo app-doc/doxygen app-editors/vim sys-apps/fakeroot\
    sys-apps/which sys-devel/gcc sys-devel/libtool sys-devel/make\
    dev-util/pkgconf dev-vcs/git dev-lang/swig dev-lang/perl dev-lang/ruby\
    dev-lang/python dev-lang/lua dev-lang/php dev-lang/tcl &&\
    sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php/*/php.ini &&\
    useradd $USER -u $UID -m -G users,wheel,portage &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
# 'dot' application fails: emerge media-gfx/graphviz
# 'epstopdf' fails: emerge app-text/texlive-core
# 'go' lang fails: emerge dev-lang/go
# 'phpunit' fails: emerge dev-php/phpunit
LABEL org.opencontainers.image.description "RPM build for libptpmgmt"
ARG COOKIE
RUN emerge --oneshot -tv sys-apps/portage
ARG SRC
ARG DST
COPY --chown=builder $SRC $DST
