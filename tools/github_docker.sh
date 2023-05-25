#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Log into github docker
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 local -r f=~/.gh.token
 if [[ -f $f ]]; then
   local server namespace
   . tools/github_params
   local -r a="$(cat $f)"
   local -r u=${a/:*/} p=${a/*:/}
   echo $p | docker login $server -u $u --password-stdin
 else
   echo "No token for github"
 fi
}
main "$@"
