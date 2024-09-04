#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# script to create Docker container for building and create archlinux packages
###############################################################################
main()
{
  local -r base_dir="$(dirname "$(realpath "$0")")"
  local -r d_user=builder2
  local -r d_dock_file=Dockerfile.github
  cd "$base_dir/.."
  source tools/make_docker.sh
  make_docker portage "$@"
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/gentoo portage

# With '-d'
docker pull ghcr.io/erezgeva/portage
docker run -it -w /home/builder2/libptpmgmt -u builder2\
  -v $(realpath .):/home/builder2/gentoo portage

# See https://wiki.gentoo.org/wiki/Gentoo_Cheat_Sheet
emerge world -ep
qlist -Iv
ls /var/db/pkg/*
}
