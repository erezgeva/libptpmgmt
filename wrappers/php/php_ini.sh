#!/bin/bash -e
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
local -r base_dir="$(dirname "$(realpath "$0")")"
cat << EOF > $base_dir/php.ini
[PHP]
extension=$base_dir/ptpmgmt.so
extension=$base_dir/clkmgr.so
EOF
}
main "$@"
