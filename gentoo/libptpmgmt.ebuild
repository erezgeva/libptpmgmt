# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Gentoo script to build libpmc
###############################################################################

EAPI=8
DESCRIPTION="PTP management library, to communicate with ptp4l"
HOMEPAGE="https://${PN}.nwtime.org"
# #!#SRC_URI="https://github.com/erezgeva/${PN}/archive/refs/tags/${PV}.tar.gz
# #!#	-> ${P}.tar.gz"
LICENSE="LGPL-3+"
SLOT="0"
# CHOST build dependencies, on installed system
DEPEND="dev-lang/perl dev-lang/ruby dev-lang/python dev-lang/lua dev-lang/php
	dev-lang/tcl"
# Fail to install: dev-lang/go
# CBUILD build dependencies, on build system
BDEPEND="sys-devel/gcc sys-devel/libtool sys-apps/which sys-devel/make
	dev-lang/swig app-doc/doxygen"
# runtime dependencies
RDEPEND="${DEPEND}"
src_configure() {
	autoconf
	econf
}
src_compile() {
	emake PMC_USE_LIB=so all
	# Fail to install a package containing: epstopdf
	# doxygen
}
# We copy the src localy, so we can debug current code :-)
RESTRICT="fetch"
SRC_URI="${P}.txz"
