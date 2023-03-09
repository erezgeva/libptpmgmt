#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# script to create Docker contianer for building and create archlinux packages
###############################################################################
cmd()
{
  echo "$@"
  "$@"
}
clean_cont()
{
  docker ps -a -f "ancestor=$1" -q | xargs -r docker rm -f
}
clean_unused_images()
{
  # Remove unnamed images
  docker images -f 'dangling=true' -q | xargs -r docker image rm -f
}
make_all_args()
{
  local arg
  for arg in user src uid cookie
  do local -n d=$arg;args+=" --build-arg ${arg^^}=$d";done
}
main()
{
  local -r base_dir="$(dirname $(realpath $0))"
  local -r cfile="$base_dir/.upgrade_cockie"
  local -r name=portage
  local -r user=builder
  local -r src=.
  local -r uid=$(id -u)
  cd "$base_dir/.."
  while getopts 'nu' opt; do
    case $opt in
      n)
        local -r no_cache=--no-cache
        ;;
      u)
        local -r upgrade=yes
        ;;
    esac
  done
  clean_cont $name
  if [[ -n "$upgrade" ]] || ! [[ -f $cfile ]]; then
    head -c200 /dev/urandom | tr -dc 'a-zA-Z0-9' |\
      fold -w 16 | head -n 1 > "$cfile"
  fi
  local -r cookie=$(cat "$cfile")
  make_all_args
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$user/" $base_dir/Dockerfile
  cmd docker build $no_cache -f $base_dir/Dockerfile $args -t $name .
  clean_unused_images
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/gentoo portage

emerge world -ep
qlist -Iv
ls /var/db/pkg/*
}
