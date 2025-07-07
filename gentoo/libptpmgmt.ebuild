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

# For eautoreconf
inherit autotools

DESCRIPTION="PTP management library, to communicate with ptp4l"
HOMEPAGE="https://${PN}.nwtime.org"
#!#!#SRC_URI="https://github.com/erezgeva/${PN}/archive/refs/tags/${PV}.tar.gz
#!#!#	-> ${P}.tar.gz"
RESTRICT="fetch"
SRC_URI="${P}.txz"
LICENSE="LGPL-3+"
SLOT="0"
IUSE="skip_perl skip_python skip_ruby skip_lua skip_php skip_tcl skip_go
	skip_swig skip_doxygen systemd"
REQUIRED_USE="skip_swig? ( skip_perl skip_python skip_ruby skip_lua skip_php
	skip_tcl skip_go )"
DEPEND="!skip_perl? ( dev-lang/perl ) !skip_python? ( dev-lang/python )
	!skip_ruby? ( dev-lang/ruby ) !skip_lua? ( dev-lang/lua )
	!skip_php? ( dev-lang/php ) !skip_tcl? ( dev-lang/tcl )
	!skip_go? ( dev-lang/go )
	|| ( dev-libs/openssl dev-libs/libgcrypt net-libs/gnutls dev-libs/nettle )"
BDEPEND="sys-devel/gcc dev-build/libtool sys-apps/which dev-build/make
	|| ( app-admin/chrpath dev-util/patchelf )
	!skip_doxygen? ( app-text/doxygen ) !skip_swig? ( dev-lang/swig )"
RDEPEND="${DEPEND}"
src_prepare() {
	default
	eautoreconf
}
src_configure() {
	local opts
	if use skip_swig; then
		opts+=" --without-swig"
	else
		if use skip_perl; then opts+=" --without-perl5"; fi
		if use skip_python; then opts+=" --without-python3"; fi
		if use skip_ruby; then opts+=" --without-ruby"; fi
		if use skip_lua; then opts+=" --without-lua"; fi
		if use skip_php; then opts+=" --without-php"; fi
		if use skip_tcl; then opts+=" --without-tcl"; fi
		if use skip_go; then opts+=" --without-go"; fi
	fi
	if use systemd; then opts+=" --enable-systemd"; fi
	econf $opts --enable-openrc
}
src_compile() {
	if use skip_doxygen; then
		emake PMC_USE_LIB=so all
	else
		emake PMC_USE_LIB=so all doxygen
	fi
}
src_install() {
	emake DESTDIR="${D}" DOC_PKG="${PF}" DEV_PKG="${PF}/dev" install compile_py3
}
