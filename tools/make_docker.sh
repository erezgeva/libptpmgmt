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
  local use_github use_gitlab use_t server namespace project
  local github_srv_ns gitlab_srv_ns
  local -r uid=$(id -u)
  local -r user=builder
  while getopts 'nuglb:f:t:' opt; do
    case $opt in
      n)
        no_cache=--no-cache
        ;;
      u) # Force update by generating a new cockie
        local -r upgrade=yes
        ;;
      g) # To the Github Server
        use_github=yes
        ;;
      l) # To the Gitlab Server
        use_gitlab=yes
        ;;
      b) # Use docker file and image with '.$use_b'
        use_b=$OPTARG
        ;;
      f) # From a Server (Github or Gitlab)
        use_f=$OPTARG
        ;;
      t) # To a Server (Github or Gitlab)
        use_t=$OPTARG
        ;;
    esac
  done
  . tools/github_params
  github_srv_ns=$server/$namespace
  . tools/gitlab_params
  gitlab_srv_ns=$server/$namespace/$project
  if [[ -n "$use_github" ]]; then
    use_srv=yes
    srv_ns=$github_srv_ns
  elif [[ -n "$use_gitlab" ]]; then
    use_srv=yes
    srv_ns=$gitlab_srv_ns
  else
    case $use_t in
      g|github)
        use_srv=yes
        srv_ns=$github_srv_ns
        ;;
      l|gitlab)
        use_srv=yes
        srv_ns=$gitlab_srv_ns
        ;;
    esac
  fi
  dock_file="$base_dir/Dockerfile"
  if [[ -n "$use_b" ]] && [[ -f "$dock_file.$use_b" ]]; then
    dock_file+=.$use_b
  else
    use_b=''
  fi
  case $use_f in
    g|github)
      use_f=$github_srv_ns/
      ;;
    l|gitlab)
      use_f=$gitlab_srv_ns/
      ;;
    *)
      use_f=''
  esac
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
  sed -i "s/^COPY --chown=[^ ]*/COPY --chown=$user/" "$dock_file"
}
make_docker()
{
  local name="$1"
  shift
  local no_cache use_srv srv_ns args dock_file use_b use_f
  tools/def_params.sh
  tool_docker_get_opts "$@"
  [[ -z "$use_b" ]] || name+=".$use_b"
  if [[ -z "$use_srv" ]]; then
    clean_cont $name
    local -r fname=$name
  else
    local -r fname=$srv_ns/$name:latest
  fi
  [[ -z "$use_f" ]] || sed -i "s!^FROM !FROM $use_f!" "$dock_file"
  cmd docker build $no_cache -f "$dock_file" $args -t $fname .
  [[ -z "$use_f" ]] || sed -i "s!^FROM $use_f!FROM !" "$dock_file"
  if [[ -n "$use_srv" ]]; then
    cmd docker push $fname
  fi
  clean_unused_images
}
cd "$base_dir/.."
