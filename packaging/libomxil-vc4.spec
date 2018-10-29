Name:          libomxil-vc4
Version:       0.0.1
Release:       2
Summary:       Libraries for interfacing to Raspberry Pi GPU
Group:         System/Libraries
URL:           https://github.com/raspberrypi/userland
Source:         %{name}-%{version}.tar.gz
License:       BSD
BuildRequires: glibc-devel
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: pkgconfig(libtbm)
ExclusiveArch: %{arm}

%description
Libraries for interfacing to Raspberry Pi GPU.

%package -n	libomxil-vc4-utils
Group:         System/Tools
Summary:       System tools for the Raspberry Pi

%description -n libomxil-vc4-utils
This package contains some system tools for the Raspberry Pi.
Source: https://github.com/libomxil-vc4/userland.git

%package -n libomxil-vc4-devel
Group:         Development/Libraries
Summary:       Development files for the Raspberry Pi GPU

%description -n libomxil-vc4-devel
This package contains libraries and header files for developing applications that use Raspberry Pi GPU.

%prep
%setup -q

%build
BUILDTYPE=Release
BUILDSUBDIR=`echo $BUILDTYPE | tr '[A-Z]' '[a-z]'`;
mkdir -p build/armv7l-linux/$BUILDSUBDIR
pushd build/armv7l-linux/$BUILDSUBDIR
cmake -DCMAKE_BUILD_TYPE=Release ../../../
make
popd

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%install
mkdir -p %{buildroot}/etc/ld.so.conf.d/
cp %{_builddir}/%{name}-%{version}/packaging/libomxil-vc4.conf %{buildroot}/etc/ld.so.conf.d
mkdir -p %{buildroot}/opt/vc/lib/plugins
mkdir %{buildroot}/opt/vc/lib/pkgconfig
mkdir %{buildroot}/opt/vc/bin
mkdir -p %{buildroot}/opt/vc/include/interface
pushd %{buildroot}/opt/vc/lib
cp %{_builddir}/%{name}-%{version}/build/lib/lib*.so ./
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/bcm_host.pc ./pkgconfig
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/brcmegl.pc ./pkgconfig
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/brcmglesv2.pc ./pkgconfig
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/brcmvg.pc ./pkgconfig
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/mmal.pc ./pkgconfig
cp %{_builddir}/%{name}-%{version}/build/armv7l-linux/release/vcsm.pc ./pkgconfig
cd ./plugins
cp %{_builddir}/%{name}-%{version}/build/lib/reader_*.so ./
cp %{_builddir}/%{name}-%{version}/build/lib/writer_*.so ./
cd ../../bin
cp %{_builddir}/%{name}-%{version}/build/bin/* ./

cd ../lib
cp %{_builddir}/%{name}-%{version}/build/lib/lib*.a ./
cd ../include
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/EGL ./
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/GLES ./
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/GLES2 ./
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/KHR ./
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/VG ./
cp -a %{_builddir}/%{name}-%{version}/interface/khronos/include/WF ./
cp -a %{_builddir}/%{name}-%{version}/interface/vmcs_host/khronos/IL ./
cp -a %{_builddir}/%{name}-%{version}/interface/mmal ./interface/
cp -a %{_builddir}/%{name}-%{version}/interface/vchi ./interface/
cp -a %{_builddir}/%{name}-%{version}/interface/vchiq_arm ./interface/
cp -a %{_builddir}/%{name}-%{version}/interface/vcos ./interface/
cp -a %{_builddir}/%{name}-%{version}/interface/vctypes ./interface/
cp -a %{_builddir}/%{name}-%{version}/interface/vmcs_host ./interface/
cp -a %{_builddir}/%{name}-%{version}/vcinclude ./
cp %{_builddir}/%{name}-%{version}/host_applications/linux/libs/bcm_host/include/bcm_host.h ./
popd

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files -n libomxil-vc4-utils
/opt/vc/bin/*
%doc LICENCE

%files -n libomxil-vc4
%manifest packaging/%{name}.manifest
%defattr(-,root,root)
%license LICENCE
/opt/vc/lib/lib*.so
/opt/vc/lib/plugins/*.so
/etc/ld.so.conf.d/libomxil-vc4.conf

%files -n libomxil-vc4-devel
/opt/vc/lib/lib*.a
/opt/vc/include/EGL
/opt/vc/include/GLES
/opt/vc/include/GLES2
/opt/vc/include/IL
/opt/vc/include/KHR
/opt/vc/include/VG
/opt/vc/include/WF
/opt/vc/include/interface
/opt/vc/include/vcinclude
/opt/vc/include/*.h
/opt/vc/lib/pkgconfig/*.pc
