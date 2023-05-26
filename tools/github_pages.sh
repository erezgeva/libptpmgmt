#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# CI (Continous integration) script for containers
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 make config
 make doxygen
 cp -a doc/html _site
}
main "$@"
