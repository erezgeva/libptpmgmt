#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# script to create Docker contianer for building and create archlinux packages
###############################################################################
main()
{
  local -r base_dir="$(dirname "$(realpath "$0")")"
  cd "$base_dir/.."
  source tools/make_docker.sh
  make_docker portage "$@"
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/gentoo portage

docker pull ghcr.io/erezgeva/portage
docker run -it -w /tmp\
  -v $(realpath ..):/tmp/libptpmgmt ghcr.io/erezgeva/portage
useradd builder2 -u 1000 -m -G users,wheel,portage
su -l builder2
cd /tmp/libptpmgmt

emerge world -ep
qlist -Iv
ls /var/db/pkg/*
}
