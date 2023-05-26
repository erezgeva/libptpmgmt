#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Log into github docker
###############################################################################
dlog()
{
 echo $2 | docker login $server -u $1 --password-stdin
}
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 local -r f=~/.gh.token
 if [[ -f $f ]]; then
   local server namespace
   . tools/github_params
   local -r a="$(cat $f)"
   dlog "${a/:*/}" "${a/*:/}"
 elif [[ -n "$1" ]] && [[ -n "$2" ]]; then
   dlog "$1" "$2"
 else
   echo "No token for github"
 fi
}
main "$@"
