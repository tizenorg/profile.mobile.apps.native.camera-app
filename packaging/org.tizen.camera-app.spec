Name:       org.tizen.camera-app
Summary:    camera application
Version:    0.0.36
Release:    1
Group:      Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

#BuildRequires: build-info
BuildRequires: cmake
BuildRequires: edje-tools
BuildRequires: gettext-tools
BuildRequires: hash-signer
BuildRequires: prelink
#BuildRequires: sec-product-features
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(cairo)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-preference)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-media-audio-io)
BuildRequires: pkgconfig(capi-media-camera)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(capi-media-recorder)
BuildRequires: pkgconfig(capi-media-sound-manager)
BuildRequires: pkgconfig(capi-media-wav-player)
BuildRequires: pkgconfig(capi-location-manager)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
#BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-telephony)
BuildRequires: pkgconfig(notification)

#Requires(post): signing-client

#Provides : libarcplatform_arm.so
#Provides : libarcsoft_panorama_burstcapture.so
#Provides : libarcpicbest_arm.so
#Provides : libarcsoft_flawlessface.so
#Provides : libarcsoft_skin_soften.so

%description
Description: org.tizen.camera-app

%prep
%setup -q

%define _optdir	/opt
%define _usrdir	/usr
%define _appdir	%{_usrdir}/apps/%{name}

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{_appdir} \

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install
#execstack -c %{buildroot}%{_appdir}/bin/camera
%define tizen_sign 1
%define tizen_sign_base /usr/apps/org.tizen.camera-app
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post
chown -R 5000:5000 /usr/apps/org.tizen.camera-app/shared
chown 5000:5000 /opt/usr/apps/org.tizen.camera-app/data/.camera.ini
ln -sf %{_appdir}/bin/camera %{_appdir}/bin/camera-appcontrol

%files
%manifest org.tizen.camera-app.manifest
%defattr(-,root,root,-)
%{_usrdir}/share/icons/default/small/org.tizen.camera-app.png
%{_usrdir}/share/packages/org.tizen.camera-app.xml
/etc/smack/accesses.d/org.tizen.camera-app.efl
%{_appdir}/bin/camera
%{_appdir}/res/edje/*
%{_appdir}/res/locale/*
%{_appdir}/res/sounds/*
%{_appdir}/shared/res/*
%{_appdir}/author-signature.xml
%{_appdir}/signature1.xml
/opt/usr/apps/org.tizen.camera-app/data/.camera.ini

#%ifarch %{arm}
#%{_appdir}/lib/*
#%endif

