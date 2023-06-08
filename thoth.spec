Summary: Thoth service
Name: thoth
Version: 0.1.0
Release: 1
License: GPLv2
Source: %{expand:%%(pwd)}
BuildRoot: %{_topdir}/BUILD/%{name}-%{version}-%{release}

%description
%{summary}

%prep
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/etc/systemd/system
mkdir -p $RPM_BUILD_ROOT/var/thoth
cd $RPM_BUILD_ROOT
cp -f %{SOURCEURL0}/thothd ./usr/bin/thothd
cp -f %{SOURCEURL0}/thothd.service ./etc/systemd/system/thothd.service
%clean
rm -r -f "$RPM_BUILD_ROOT"

%files
%defattr(755,root,root)
/usr/bin/thothd
%defattr(644,root,root)
/etc/systemd/system/thothd.service
%dir /var/thoth
