Summary: Colorful X solitaire Mah Jongg game

Name: xmahjongg
Version: 3.7
Release: 1
Source: http://www.lcdf.org/xmahjongg/xmahjongg-3.7.tar.gz

Icon: logo.gif
URL: http://www.lcdf.org/xmahjongg/

Group: X11/Games
Vendor: Little Cambridgeport Design Factory
Packager: Eddie Kohler <eddietwo@lcs.mit.edu>
Copyright: GPL

BuildRoot: /tmp/xmahjongg-build

%description
Real Mah Jongg is a social game that originated in
China thousands of years ago. Four players, named
after the four winds, take tiles from a wall in
turn. The best tiles are made of ivory and wood;
they click pleasantly when you knock them
together. Computer Solitaire Mah Jongg (xmahjongg
being one of the sillier examples) is nothing like
that but it's fun, or it must be, since there are
like 300 shareware versions available for Windows.
This is for X11 and it's free.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --datadir=/usr/X11R6/lib/X11
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/X11R6/bin $RPM_BUILD_ROOT/usr/X11R6/man/man6
install -c -s xmahjongg $RPM_BUILD_ROOT/usr/X11R6/bin/xmahjongg
install -c -m 644 xmahjongg.6 $RPM_BUILD_ROOT/usr/X11R6/man/man6/xmahjongg.6
make pkgdatadir="$RPM_BUILD_ROOT/usr/X11R6/lib/X11/xmahjongg" install-share

%clean
rm -rf $RPM_BUILD_ROOT

%post

%files
%attr(-,root,root) %doc NEWS README
%attr(0755,root,root) /usr/X11R6/bin/xmahjongg
%attr(0644,root,root) /usr/X11R6/man/man6/xmahjongg.6*
%attr(-,root,root) /usr/X11R6/lib/X11/xmahjongg
