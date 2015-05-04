Name:       wifi-native
Summary:    Tizen crosswalk extension for wifi
Version:    1.0.0
Release:    1
Group:      Applications/System
License:    Apache-2.0
URL:        http://www.tizen.org2
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  common-native
BuildRequires:  desktop-file-utils

Requires: pkgmgr
Requires: crosswalk
Requires: tizen-extensions-crosswalk
Requires: pkgmgr-server
Requires: model-config-ivi
Requires: tizen-middleware-units
Requires: tizen-platform-config

%description
Tizen crosswalk extension for wifi, providing apps with a convenient api to
connman.

%prep
%setup -q -n %{name}-%{version}

%build
make

%install
rm -rf %{buildroot}
make install_obs "OBS=1" DESTDIR="%{?buildroot}"

%files
%defattr(-,root,root,-)

