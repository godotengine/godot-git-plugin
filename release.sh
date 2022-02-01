#!/bin/bash

# This script is used to create releases for this plugin. This is mostly a helper script for the maintainer.

echo "Enter the new version number (e.g. 1.2.1):"
read version

echo "Enter the Windows x64 release ZIP URL:"
read windowsZIPURL

echo "Enter the Linux x64 release ZIP URL:"
read linuxZIPURL

echo "Enter the MacOS universal release ZIP URL:"
read macZIPURL

# wget-ing the github.com URL gives a 404, so we use the method proposed here - https://github.com/actions/upload-artifact/issues/51#issuecomment-735989475
windowsZIPURL=${windowsZIPURL/github.com/nightly.link}
linuxZIPURL=${linuxZIPURL/github.com/nightly.link}
macZIPURL=${macZIPURL/github.com/nightly.link}

wget -O windows.zip $windowsZIPURL
wget -O x11.zip $linuxZIPURL
wget -O mac.zip $macZIPURL

unzip windows.zip -d windows/
unzip x11.zip -d x11/
unzip mac.zip -d mac/

releasePath=godot-git-plugin-v$version
mkdir $releasePath

cp -r windows/addons/ $releasePath
addonsPath=$releasePath/addons
pluginPath=$addonsPath/godot-git-plugin

mkdir $pluginPath/x11
mkdir $pluginPath/osx
cp -r x11/addons/godot-git-plugin/x11/ $pluginPath/
cp -r mac/addons/godot-git-plugin/osx/ $pluginPath/

sed -i "s/version=\"[^\"]*\"/version=\"v${version}\"/g" $pluginPath/plugin.cfg
cp LICENSE $pluginPath/LICENSE
cp THIRDPARTY.md $pluginPath/THIRDPARTY.md

rm -f $pluginPath/x11/libgit2.a
rm -f $pluginPath/osx/libgit2.a
rm -f $pluginPath/win64/git2.lib

pushd $releasePath
zip -r $releasePath.zip addons/
popd

mv $releasePath/$releasePath.zip ./

rm -rf $releasePath
rm -rf windows
rm -rf x11
rm -rf mac
rm windows.zip
rm x11.zip
rm mac.zip
