# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Docker file for Gentoo container for building and installing
# Create from Docker image from base image
# https://dev.gentoo.org/~zmedico/portage/doc/man/emerge.1.html
# --tree, -t            Shows the dependency tree
# --verbose, -v         Verbose mode
# --update, -u          Updates packages to the best version available
# --deep [DEPTH], -D    Entire dependency tree of packages
# --sync                Updates repositories
# --depclean, -c        Cleans the system by removing packages
#                       that are not associated with explicitly merged packages
# --fetchonly, -f       Fetches for all packages based upon USE setting
# --color n             Disable color
###############################################################################
FROM portage.2
ARG UID USER
RUN useradd $USER -u $UID -m -G users,wheel,portage
ARG COOKIE
RUN emerge --sync && emerge -tvuD @world
ARG SRC DST
COPY --chown=builder $SRC $DST
