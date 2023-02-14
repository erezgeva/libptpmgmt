#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
# Create ini file for testing.
###############################################################################
main()
{
cat << EOF > ${1}php.ini
[PHP]
extension=$PWD/${1}ptpmgmt.so
EOF
}
main "$@"
