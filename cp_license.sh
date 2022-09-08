#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2022 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2022 Erez Geva
#
# Follow FSF RESUSE Specification https://reuse.software/spec/
# Provide folders with all licenses
###############################################################################
down_lic()
{
  if [[ -f "LICENSES/$1.txt" ]] || [[ -z "$(which reuse)" ]]; then
    return
  fi
  reuse download $1
}
main()
{
 mkdir -p LICENSES
 local n
 for n in GPL-3.0-or-later LGPL-3.0-or-later BSD-3-Clause\
          GFDL-1.3-no-invariants-or-later GPL-2.0-or-later MIT
 do down_lic $n; done
}
cd $(dirname $(realpath $0))
main
