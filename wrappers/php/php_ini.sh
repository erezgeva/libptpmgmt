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
echo '[PHP]' > $base_dir/php.ini
local n
for n in ptpmgmt $@
do echo "extension=$base_dir/$n.so" >> $base_dir/php.ini;done
}
main "$@"
