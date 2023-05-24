#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Create release spec for current version with official release from github.
#
# Download from:
#  spectool -g -R --define "_topdir $PWD" libptpmgmt-*.spec
# Build it:
#  rpmbuild --define "_topdir $PWD" -bb libptpmgmt-*.spec
#
###############################################################################
main()
{
  local ver_maj ver_min
  . ../tools/version
  local -r version="$ver_maj.$ver_min"
  sed '/^Source0:/ d;  /#Source0:/ { s/%%/%/g;s/^#// }' libptpmgmt.spec >\
    libptpmgmt-${version}.spec
}
if [[ -f libptpmgmt.spec ]] && [[ -f ../tools/version ]]; then
  main
fi
