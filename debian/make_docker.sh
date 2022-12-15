#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# script to create Docker contianer for building and create Debian packages
###############################################################################
cmd()
{
  echo "$@"
  "$@"
}
clean_cont()
{
  local -r n=$1
  docker ps -a -f "ancestor=$n" -q | xargs -r docker rm -f
}
clean_unused_images()
{
  # Remove unnamed images
  docker images -f 'dangling=true' -q | xargs -r docker image rm -f
}
make_all_args()
{
  local arg
  for arg in USER SRC ARCHS
  do local -n d=$arg;args+=" --build-arg $arg=$d";done
  args+=" --build-arg UID=$uid"
}
set_dist_args()
{
  SRC_CFG="deb $repo $1 main\ndeb $repo $1-updates main\n"
  case $1 in
    buster|stretch)
      SRC_CFG+="deb $repo-security $1/updates main\n"
      ;;
    *)
      SRC_CFG+="deb $repo-security $1-security main\n"
      ;;
  esac
  DPKGS="$DPKGS_all"
  local -n d=DPKGS_$1
  local m
  for m in $d; do
    if [[ $m =~ @$ ]]; then
      # Package per architecture
      local p=${m%@}
      DPKGS+=" $p:$main_arch"
      for a in $ARCHS; do
        DPKGS+=" $p:$a"
      done
    else
      DPKGS+=" $m"
    fi
  done
}
main()
{
  local a n SRC_CFG DPKGS
  local -r repo=http://ftp.de.debian.org/debian
  local -r base_dir=$(dirname $(realpath $0))
  local -r bname=deb.
  local -r names='stretch buster bullseye bookworm'
  local -r main_arch=$(dpkg --print-architecture) # amd64
  local -r ARCHS='arm64'
  local -r USER=builder
  local -r lua54='lua5.4 liblua5.4-dev@'
  local -r DPKGS_stretch='vim-gtk'
  local -r DPKGS_buster='vim-gtk'
  local -r DPKGS_bullseye="vim-gtk $lua54"
  local -r DPKGS_bookworm="reuse vim-gtk3 $lua54"
  # Packages per architecture
  for n in libstdc++6 liblua5.1-0-dev liblua5.2-dev liblua5.3-dev\
           libpython3-all-dev ruby-dev tcl-dev libpython3-dev\
           libfastjson-dev libgtest-dev
  do
    # Main architecture
    DPKGS_all+=" $n:$main_arch"
    for a in $ARCHS; do
      DPKGS_all+=" $n:$a"
    done
  done
  for a in $ARCHS; do
    n="$(dpkg-architecture -a$a -qDEB_TARGET_GNU_TYPE 2> /dev/null)"
    DPKGS_all+=" g++-$n"
  done
  local -r SRC=.
  local -r uid=$(id -u)
  cd $base_dir/..
  while getopts 'n' opt; do
    case $opt in
      n)
        local -r no_cache=--no-cache
        ;;
    esac
  done
  for n in $names; do clean_cont $bname$n; done
  make_all_args
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$USER/" $base_dir/Dockerfile
  for n in $names; do
    set_dist_args $n
    cmd docker build $no_cache -f $base_dir/Dockerfile $args\
        --build-arg DIST=$n --build-arg SRC_CFG="$SRC_CFG" \
        --build-arg DPKGS="$DPKGS"\
        -t $bname$n .
  done
  clean_unused_images
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.stretch
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.buster
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.bullseye
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.bookworm
}
