#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Create release PKGBUILD, using official version release from github.
###############################################################################
main()
{
  head -n -2 PKGBUILD.org | sed '/^#source=/ s/^#//' > PKGBUILD
  makepkg -g >> PKGBUILD
}
if [[ -f PKGBUILD.org ]] && [[ -x "$(which makepkg)" ]]; then
  main
fi
