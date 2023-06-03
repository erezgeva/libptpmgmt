#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Script to run AddressSanitizer in GitHub
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 sudo apt-get install -y --no-install-recommends libtool libtool-bin
 autoconf
 ./configure
 tools/sim_ptp4l.sh -a
}
main "$@"
