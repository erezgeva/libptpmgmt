# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
# @copyright GNU General Public License 3.0 or later
#
# Docker file for archlinux contianer for building and create packages
###############################################################################
FROM archlinux
ARG UID
ARG USER
RUN patched_glibc=glibc-linux4-2.33-4-x86_64.pkg.tar.zst &&\
    curl -LO "https://repo.archlinuxcn.org/x86_64/$patched_glibc" &&\
    bsdtar -C / -xvf "$patched_glibc" &&\
    pacman -Syu --noconfirm --needed bash rustup gcc fakeroot pkgconf libtool\
    which make git sudo doxygen graphviz perl ruby swig python2 python3\
    lua lua51 lua52 lua53 vim &&\
    useradd $USER -u $UID -m -G users,wheel &&\
    echo "$USER ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers
USER $USER
ARG SRC
COPY --chown=builder $SRC /home/$USER/libpmc
