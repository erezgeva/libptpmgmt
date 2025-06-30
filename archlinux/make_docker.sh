#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# script to create Docker container for building and create Arch Linux packages
###############################################################################
main()
{
  local -r base_dir="$(dirname "$(realpath "$0")")"
  source "$base_dir/../tools/make_docker.sh"
  make_docker pacmanbuild "$@"
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/archlinux pacmanbuild
}
