Summary: Thoth service
Name: thoth
Version: 0.0.0
Release: 1
License: ASL 2.0
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
cp -f %{SOURCEURL0}/thoth ./usr/bin/thoth
cp -f %{SOURCEURL0}/thothd.service ./etc/systemd/system/thothd.service
%clean
rm -r -f "$RPM_BUILD_ROOT"

%files
%defattr(755,root,root)
/usr/bin/thothd
%defattr(644,root,root)
/etc/systemd/system/thothd.service
/usr/bin/thoth
%dir /var/thoth
