# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Gentoo script to build libpmc
# See: https://devmanual.gentoo.org/ebuild-writing/functions/index.html
###############################################################################

EAPI=8

# For eautoconf
inherit autotools

DESCRIPTION="PTP management library, to communicate with ptp4l"
HOMEPAGE="https://${PN}.nwtime.org"
#!#!#SRC_URI="https://github.com/erezgeva/${PN}/archive/refs/tags/${PV}.tar.gz
#!#!#	-> ${P}.tar.gz"
RESTRICT="fetch"
SRC_URI="${P}.txz"
LICENSE="LGPL-3+"
SLOT="0"
DEPEND="dev-lang/perl dev-lang/ruby dev-lang/python dev-lang/lua dev-lang/php
	dev-lang/tcl dev-lang/go"
BDEPEND="sys-devel/gcc dev-build/libtool sys-apps/which dev-build/make
	dev-lang/swig app-text/doxygen"
RDEPEND="${DEPEND}"
src_prepare() {
	default
	eautoconf
}
src_compile() {
	emake PMC_USE_LIB=so all doxygen
}
src_install() {
	emake DESTDIR="${D}" install
	# Move documents to Gentoo location
	mv "${D}/usr/share/doc/${PN}-doc" "${D}/usr/share/doc/${PF}"
	# TODO python byte-compiled module: /usr/lib/python*/site-packages/ptpmgmt.py
}
