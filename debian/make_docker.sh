#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# script to create Docker container for building and create Debian packages
###############################################################################
set_dist_args()
{
  dpkgs="$dpkgs_all"
  local -n d=dpkgs_$1
  for m in $d; do
    if [[ $m =~ @$ ]]; then
      # Package per architecture
      p=${m%@}
      dpkgs+=" $p:$arch"
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
  local -r base_dir="$(dirname "$(realpath "$0")")"
  source "$base_dir/../tools/make_docker.sh"
  local -r repo=http://ftp.de.debian.org/debian
  local -r names='bookworm trixie'
  local -r arch=$(dpkg --print-architecture) # amd64
  local -r archs='arm64'
  local -r dpkgs_bookworm=''
  local -r dpkgs_trixie='librtpi-dev@ libgcrypt20-dev@'
  local dpkgs_arch='libstdc++6 pkgconf
    libpython3-all-dev ruby-dev tcl-dev libpython3-dev libperl-dev
    libfastjson-dev libgtest-dev libgmock-dev lua-posix libjson-c-dev
    libssl-dev libgcrypt20 libgnutls28-dev nettle-dev libsystemd-dev'
  for n in 1-0 {2..4};do dpkgs_arch+=" liblua5.$n-dev";done
  local no_cache use_srv srv_ns args dock_file use_b use_f
  tool_docker_get_opts "$@"
  if [[ -z "$use_srv" ]]; then
    for n in $names; do clean_cont $bname$n; done
    local -r bname=deb.
    local -r ename=
  else
    local -r bname=$srv_ns/deb.
    local -r ename=:latest
  fi
  [[ -z "$use_b" ]] || ename+=".$use_b"
  local a n m p
  # Packages per architecture
  for n in $dpkgs_arch; do
    # Main architecture
    dpkgs_all+=" $n:$arch"
    for a in $archs; do
      dpkgs_all+=" $n:$a"
    done
  done
  for a in $archs; do
    n="$(dpkg-architecture -a$a -qDEB_TARGET_GNU_TYPE 2> /dev/null)"
    dpkgs_all+=" g++-$n"
  done
  local dpkgs all_args="$args"
  make_args repo arch
  all_args+="$args"
  for dist in $names; do
    make_args dist
    set_dist_args $dist
    cmd docker build $no_cache -f "$dock_file" $all_args $args\
        --build-arg ARCHS="$archs"\
        --build-arg DPKGS="$dpkgs" -t $bname$dist$ename .
    if [[ -n "$use_srv" ]]; then
      cmd docker push $bname$dist$ename
    fi
  done
  clean_unused_images
}
main "$@"
ext()
{
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/debian deb.bookworm
docker run -it -w /home/builder/libptpmgmt -u builder\
  -v $(realpath .):/home/builder/debian deb.trixie
}
