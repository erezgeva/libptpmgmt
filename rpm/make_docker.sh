#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# script to create Docker container for building and create rpm packages
###############################################################################
main()
{
  local -r base_dir="$(dirname "$(realpath "$0")")"
  source "$base_dir/../tools/make_docker.sh"
  make_docker rpmbuild "$@"
}
main "$@"
ext()
{
# See:
# https://www.cyberciti.biz/howto/question/linux/linux-rpm-cheat-sheet.php
# See package content
rpm -qpil clkmgr-proxy-1*.x86_64.rpm
# List installed packages
rpm -qa
dnf list installed
# Find out what package a file belongs to
rpm -qf file
# See package scripts
rpm -qlp --scripts clkmgr-proxy-1*.x86_64.rpm
# Search For Packages
dnf search php

docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/rpm rpmbuild
}
