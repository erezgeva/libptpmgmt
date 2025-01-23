# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Docker file for Gentoo container for building and installing
# Create from Docker image from base image
# https://dev.gentoo.org/~zmedico/portage/doc/man/emerge.1.html
# --tree, -t Shows the dependency tree
# --verbose, -v
# --update, -u
# --deep [DEPTH], -D    entire dependency tree of packages
###############################################################################
FROM portage_base
ARG UID USER
# 559m34.815s = 9h 16m 34s
RUN emerge -tv llvm-core/clang &&\
    useradd $USER -u $UID -m -G users,wheel,portage &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
ARG COOKIE
# RUN emerge --sync && emerge -tvuD @world
ARG SRC DST
COPY --chown=builder $SRC $DST
