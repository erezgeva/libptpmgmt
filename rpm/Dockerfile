# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
# @copyright GNU General Public License 3.0 or later
#
# Docker file for Fedora contianer for building and create rpm packages
###############################################################################
FROM fedora
ARG UID
ARG USER
RUN dnf install -y gcc gcc-c++ libtool libtool-ltdl make cmake git pkgconfig\
        sudo automake autoconf yum-utils rpm-build dnf-utils fedora-packager\
        rpmdevtools doxygen graphviz perl php-devel ruby ruby-devel swig\
        lua lua-devel python2-devel python3-devel php python2 python3 which\
        perl-devel perl-ExtUtils-Embed lua-posix && dnf clean all &&\
        useradd $USER -u $UID -m -G users,wheel &&\
        echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers &&\
        sed -i 's/^enable_dl\s*=\s*Off/enable_dl = On/' /etc/php.ini
ARG SRC
COPY $SRC /home/$USER/libpmc
RUN chown -R $USER.$USER /home/$USER/libpmc
USER $USER
