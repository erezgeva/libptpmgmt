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
  local -r b_dock_file=Dockerfile.base
  local -r b_name=portage_base
  cd "$base_dir/.."
  source tools/make_docker.sh
  make_docker portage "$@"
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/gentoo portage

docker pull ghcr.io/erezgeva/portage_base
docker tag ghcr.io/erezgeva/portage_base portage_base

# See https://wiki.gentoo.org/wiki/Gentoo_Cheat_Sheet
emerge world -ep
qlist -Iv
ls /var/db/pkg/*
}
