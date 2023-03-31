#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
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
  docker ps -a -f "ancestor=$1" -q | xargs -r docker rm -f
}
clean_unused_images()
{
  # Remove unnamed images
  docker images -f 'dangling=true' -q | xargs -r docker image rm -f
}
make_args()
{
  args=''
  for arg in "$@"
  do local -n d=$arg;args+=" --build-arg ${arg^^}=$d";done
}
set_dist_args()
{
  SRC_CFG="deb $repo $1 main\ndeb $repo $1-updates main\n"
  case $1 in
    buster)
      SRC_CFG+="deb $repo-security $1/updates main\n"
      ;;
    *)
      SRC_CFG+="deb $repo-security $1-security main\n"
      ;;
  esac
  dpkgs="$dpkgs_all"
  local -n d=dpkgs_$1
  for m in $d; do
    if [[ $m =~ @$ ]]; then
      # Package per architecture
      p=${m%@}
      dpkgs+=" $p:$main_arch"
      for a in $archs; do
        dpkgs+=" $p:$a"
      done
    else
      dpkgs+=" $m"
    fi
  done
}
main()
{
  local -r repo=http://ftp.de.debian.org/debian
  local -r base_dir="$(dirname $(realpath $0))"
  local -r cfile="$base_dir/.upgrade_cockie"
  local -r bname=deb.
  local -r names='buster bullseye bookworm'
  local -r main_arch=$(dpkg --print-architecture) # amd64
  local -r archs='arm64'
  local -r user=builder
  local -r src=.
  local -r uid=$(id -u)
  local -r lua54='lua5.4 liblua5.4-dev@'
  local -r dpkgs_buster='vim-gtk dh-golang'
  local -r dpkgs_bullseye="vim-gtk $lua54 dh-golang"
  local -r dpkgs_bookworm="reuse vim-gtk3 $lua54 dh-golang"
  local a n m p arg args
  # Packages per architecture
  for n in libstdc++6 liblua5.1-0-dev liblua5.2-dev liblua5.3-dev\
           libpython3-all-dev ruby-dev tcl-dev libpython3-dev\
           libfastjson-dev libgtest-dev
  do
    # Main architecture
    dpkgs_all+=" $n:$main_arch"
    for a in $archs; do
      dpkgs_all+=" $n:$a"
    done
  done
  for a in $archs; do
    n="$(dpkg-architecture -a$a -qDEB_TARGET_GNU_TYPE 2> /dev/null)"
    dpkgs_all+=" g++-$n"
  done
  cd $base_dir/..
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
  if [[ -n "$upgrade" ]] || ! [[ -f $cfile ]]; then
    head -c200 /dev/urandom | tr -dc 'a-zA-Z0-9' |\
      fold -w 16 | head -n 1 > "$cfile"
  fi
  local -r cookie=$(cat "$cfile")
  for n in $names; do clean_cont $bname$n; done
  make_args user src uid cookie
  local SRC_CFG dpkgs all_args="$args"
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$user/" $base_dir/Dockerfile
  for dist in $names; do
    make_args dist
    set_dist_args $dist
    cmd docker build $no_cache -f $base_dir/Dockerfile $all_args $args\
        --build-arg ARCHS="$archs"\
        --build-arg SRC_CFG="$SRC_CFG"\
        --build-arg DPKGS="$dpkgs" -t $bname$dist .
  done
  clean_unused_images
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.buster
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.bullseye
docker run -it -w /home/builder/libptpmgmt\
  -v $(realpath .):/home/builder/debian deb.bookworm
}
