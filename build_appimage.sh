#!/usr/bin/env sh

## NOTE: Claude Haiku 4.5 (free tier) has been used to debug this script and to fix some errors, please don't hate me for that :)

# delete the old build dir
sudo rm -rf ./pwscp.AppDir

# create new dir structure
mkdir -p ./pwscp.AppDir/{usr/{bin,lib,lib64,share/{applications,icons,pixmaps}},opt,etc}
mkdir -p ./pwscp.AppDir/usr/share/icons/hicolor/256x256/

# copy assets and executable
cp ./cmake-build-release/pwscp ./pwscp.AppDir/usr/bin/pwscp
cp ./Assets/pwscp.png ./pwscp.AppDir/pwscp.png
cp ./Assets/pwscp.desktop ./pwscp.AppDir/pwscp.desktop

# Create AppRun script
cat > ./pwscp.AppDir/AppRun << 'EOF'
#!/bin/sh
APPDIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${APPDIR}/usr/lib/qt6/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="${APPDIR}/usr/lib/qt6/plugins"
exec "${APPDIR}/usr/bin/pwscp" "$@"
EOF
chmod +x ./pwscp.AppDir/AppRun

# Manually copy Qt 6 libraries
mkdir -p ./pwscp.AppDir/usr/lib/qt6/plugins
cp -r /usr/lib/qt6/plugins/* ./pwscp.AppDir/usr/lib/qt6/plugins/
cp /usr/lib/libQt6*.so* ./pwscp.AppDir/usr/lib/ 2>/dev/null || true
cp /usr/lib/libicu*.so* ./pwscp.AppDir/usr/lib/ 2>/dev/null || true

# Use ldd to find remaining dependencies and copy them
ldd ./pwscp.AppDir/usr/bin/pwscp | grep "=>" | awk '{print $3}' | while read lib; do
  if [ -n "$lib" ] && [ -f "$lib" ]; then
    cp "$lib" ./pwscp.AppDir/usr/lib/ 2>/dev/null || true
  fi
done

# Create the AppImage
sudo /home/lorenzo/Downloads/appimagetool-x86_64.AppImage ./pwscp.AppDir pwscp.AppImage
