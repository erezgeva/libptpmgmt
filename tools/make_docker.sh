#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# script to create Docker contianer for building and create packages
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
  docker images -f 'dangling=true' -q | xargs -r docker rmi -f
}
make_args()
{
  args=''
  local arg
  for arg in "$@"
  do local -n d=$arg;args+=" --build-arg ${arg^^}=$d";done
}
tool_docker_get_opts()
{
  local -r user=builder
  local -r uid=$(id -u)
  while getopts 'nug' opt; do
    case $opt in
      n)
        no_cache=--no-cache
        ;;
      u)
        local -r upgrade=yes
        ;;
      g)
        use_github=yes
        ;;
    esac
  done
  if [[ -z "$use_github" ]]; then
    local -r src=.
    local -r dst=/home/$user/libptpmgmt
  else
    local server namespace
    . tools/github_params
    touch .null
    local -r src=.null
    local -r dst=/tmp
    gh_ns=$server/$namespace
  fi
  local -r cfile="$base_dir/.upgrade_cockie"
  if [[ -n "$upgrade" ]] || ! [[ -f $cfile ]]; then
    head -c200 /dev/urandom | tr -dc 'a-zA-Z0-9' |\
      fold -w 16 | head -n 1 > "$cfile"
  fi
  local -r cookie=$(cat "$cfile")
  make_args user src dst uid cookie
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$user/" "$base_dir/Dockerfile"
}
make_docker()
{
  local -r name="$1"
  shift
  local no_cache use_github gh_ns args
  tool_docker_get_opts "$@"
  if [[ -z "$use_github" ]]; then
    clean_cont $name
    local -r fname=$name
  else
    local -r fname=$gh_ns/$name:latest
  fi
  cmd docker build $no_cache -f "$base_dir/Dockerfile" $args -t $fname .
  if [[ -n "$use_github" ]]; then
    cmd docker push $fname
  fi
  clean_unused_images
}
