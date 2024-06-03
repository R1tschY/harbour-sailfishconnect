Name:       harbour-sailfishconnect

Summary:    SailfishOS client for KDE-Connect
Version:    0.7.0
Release:    1
License:    LICENSE
URL:        https://github.com/R1tschY/harbour-sailfishconnect
Source0:    %{name}-%{version}.tar.bz2
Source1:    CMakeLists.txt
Source2:    %{_target_cpu}.profile
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(openssl) >= 1.1.1 
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(Qt5Feedback)
BuildRequires:  pkgconfig(nemonotifications-qt5)
BuildRequires:  pkgconfig(keepalive)
BuildRequires:  cmake
BuildRequires:  ninja
BuildRequires:  gettext
BuildRequires:  python3-devel
BuildRequires:  python3-base
BuildRequires:  python3-setuptools
BuildRequires:  python3-pip
BuildRequires:  desktop-file-utils
BuildRequires:  extra-cmake-modules
# needed only for building dependencies
BuildRequires:  git
BuildRequires:  pkgconfig(Qt5Script)
BuildRequires:  gettext-devel

%description
SailfishOS client for KDE-Connect


%prep
%setup -q -n %{name}-%{version}


%build

VENV=.venv-conan-%{_target_cpu}
export TARGET_CPU="%{_target_cpu}"
export SAILFISHCONNECT_PACKAGE_VERSION="%{version}-%{release}"

SOURCE_DIR=`readlink -f %{_sourcedir}/..`

if [ "$TARGET_CPU" == "i486" ] ; then  
  GENERATOR="Unix Makefiles"
else
  GENERATOR="Ninja"
fi

if [ "$SAILFISH_SDK_FRONTEND" == "qtcreator" ] ; then  
  CMAKE_BUILD_TYPE="Debug"
else
  CMAKE_BUILD_TYPE="RelWithDebInfo"
fi

if [ -f "CMakeLists.txt" ] ; then
  BUILD_DIR="rpmbuilddir-%{_target_cpu}"
else 
  BUILD_DIR="."
fi
export CONAN_USER_HOME=${SOURCE_DIR}

# install virtualenv
if [ ! -f ~/.local/bin/virtualenv ] ; then
  python3 -m pip install --no-warn-script-location --user virtualenv
fi

# create virtualenv and install conan
if [ ! -f "$VENV/bin/conan" ] ; then
  ~/.local/bin/virtualenv --python=python3 "$VENV"
  source "$VENV/bin/activate"
else
  source "$VENV/bin/activate"
fi
echo "cython<3" > /tmp/pip-constraint.txt
PIP_CONSTRAINT=/tmp/pip-constraint.txt pip install conan===1.60.1

# speed up conan remote add
if ! grep -sq sailfishos "$CONAN_USER_HOME/.conan/remotes.json" ; then
  conan remote add -f sailfishos https://gitlab.com/api/v4/projects/12208535/packages/conan
fi

mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"
conan install "$SOURCE_DIR" --build=missing --profile:build="$SOURCE_DIR/dev/profiles/%{_target_cpu}" --profile:host="$SOURCE_DIR/dev/profiles/%{_target_cpu}"
conan remove -f "*" --builds --src --system-reqs

cmake \
  -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCONAN_DISABLE_CHECK_COMPILER=ON \
  -DSAILFISHOS=ON \
  -G "$GENERATOR" \
  "$SOURCE_DIR"

cmake --build . -- %{?_smp_mflags}

%install
if [ -f "CMakeLists.txt" ] ; then
  BUILD_DIR="rpmbuilddir-%{_target_cpu}"
else 
  BUILD_DIR="."
fi

rm -rf %{buildroot}
DESTDIR=%{buildroot} cmake --build "$BUILD_DIR" --target install
rm -rf \
  %{buildroot}%{_datadir}/knotifications5 \
  %{buildroot}%{_datadir}/kservicetypes5 \
  %{buildroot}%{_datadir}/qlogging-categories5 \
  %{buildroot}%{_datadir}/%{name}/lib/*.a \
  %{buildroot}%{_datadir}/locale
mkdir -p %{buildroot}%{_bindir}

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/locale
%{_datadir}/%{name}/harbour-sailfishconnect.service
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
