#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# script to create Docker container for building and create packages
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
  local use_github use_gitlab server namespace
  local -r uid=$(id -u)
  local -r user=builder
  while getopts 'nuglb' opt; do
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
      l)
        use_gitlab=yes
        ;;
      b)
        use_b=yes
        ;;
    esac
  done
  if [[ -n "$use_github" ]]; then
    . tools/github_params
    use_srv=yes
    srv_ns=$server/$namespace
  elif [[ -n "$use_gitlab" ]]; then
    . tools/gitlab_params
    use_srv=yes
    srv_ns=$server/$namespace/libptpmgmt
  fi
  if [[ -n "$use_b" ]] && [[ -n "$b_dock_file" ]]; then
    dock_file=$b_dock_file
  else
    dock_file=Dockerfile
  fi
  if [[ -n "$use_srv" ]]; then
    touch .null
    local -r src=.null
    local -r dst=/tmp
  else
    local -r src=.
    local -r dst=/home/$user/libptpmgmt
  fi
  local -r cfile="$base_dir/.upgrade_cockie"
  if [[ -n "$upgrade" ]] || ! [[ -f $cfile ]]; then
    head -c200 /dev/urandom | tr -dc 'a-zA-Z0-9' |\
      fold -w 16 | head -n 1 > "$cfile"
  fi
  local -r cookie=$(cat "$cfile")
  make_args user src dst uid cookie
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$user/" "$base_dir/$dock_file"
}
make_docker()
{
  local name="$1"
  shift
  local no_cache use_srv srv_ns args dock_file use_b
  tool_docker_get_opts "$@"
  if [[ -n "$use_b" ]] && [[ -n "$b_name" ]]; then
    name="$b_name"
  fi
  if [[ -z "$use_srv" ]]; then
    clean_cont $name
    local -r fname=$name
  else
    local -r fname=$srv_ns/$name:latest
  fi
  cmd docker build $no_cache -f "$base_dir/$dock_file" $args -t $fname .
  if [[ -n "$use_srv" ]]; then
    cmd docker push $fname
  fi
  clean_unused_images
}
