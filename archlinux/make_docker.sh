#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
# @copyright GNU General Public License 3.0 or later
#
# script to create Docker contianer for building and create archlinux packages
###############################################################################
cmd()
{
  echo $*
  $*
}
clean_cont()
{
  local -r n=$1
  docker ps -a -f "ancestor=$n" -q | xargs -r docker rm -f
}
clean_unused_images()
{
  # Remove unnamed images
  docker images -f "dangling=true" -q | xargs -r docker image rm -f
}
make_all_args()
{
  local arg
  for arg in USER SRC
  do local -n n=$arg;args+=" --build-arg $arg=$n";done
  for arg in UID=$(id -u)
  do args+=" --build-arg $arg";done
}
main()
{
  local -r base_dir=$(dirname $(realpath $0))
  local -r name=pacmanbuild
  local -r USER=builder
  local -r SRC=.
  cd $base_dir/..
  while getopts 'n' opt; do
    case $opt in
      n)
        local -r no_cache=--no-cache
        ;;
    esac
  done
  clean_cont $name
  make_all_args
  cmd docker build $no_cache -f $base_dir/Dockerfile $args -t $name .
  clean_unused_images
}
main $*
# docker run -it -v $(realpath .):/home/builder/archlinux pacmanbuild
