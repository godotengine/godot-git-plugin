#!/bin/bash

# This script is used to create releases for this plugin. This is mostly a helper script for the maintainer.

echo "Enter the new version number (e.g. 1.2.1):"
read version

echo "Enter the Windows x64 release ZIP artifact ID or URL:"
read windowsArtifactID

echo "Enter the Linux x64 release ZIP artifact ID or URL:"
read linuxArtifactID

echo "Enter the MacOS universal release ZIP artifact ID or URL:"
read macArtifactID

# wget-ing the github.com URL gives a 404, so we use the method proposed here - https://github.com/actions/upload-artifact/issues/51#issuecomment-735989475
# The ${foo##*/} syntax extracts the string after the last '/' in case it's a full artifact URL.
baseURL=https://nightly.link/godotengine/godot-git-plugin/actions/artifacts/
windowsZIPURL=${baseURL}${windowsArtifactID##*/}.zip
linuxZIPURL=${baseURL}${linuxArtifactID##*/}.zip
macZIPURL=${baseURL}${macArtifactID##*/}.zip

wget -O windows.zip $windowsZIPURL
wget -O linux.zip $linuxZIPURL
wget -O mac.zip $macZIPURL

unzip windows.zip -d windows/
unzip linux.zip -d linux/
unzip mac.zip -d mac/

releasePath=godot-git-plugin-v$version
mkdir $releasePath

cp -r windows/addons/ $releasePath
addonsPath=$releasePath/addons
pluginPath=$addonsPath/godot-git-plugin

mkdir $pluginPath/linux
mkdir $pluginPath/macos
cp -r linux/addons/godot-git-plugin/linux/ $pluginPath/
cp -r mac/addons/godot-git-plugin/macos/ $pluginPath/

sed -i "s/version=\"[^\"]*\"/version=\"v${version}\"/g" $pluginPath/plugin.cfg
cp LICENSE $pluginPath/LICENSE
cp THIRDPARTY.md $pluginPath/THIRDPARTY.md

zip -r $releasePath.zip $addonsPath

rm -rf $releasePath
rm -rf windows
rm -rf linux
rm -rf mac
rm windows.zip
rm linux.zip
rm mac.zip
