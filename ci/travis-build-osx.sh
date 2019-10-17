#!/usr/bin/env bash

#
# Build the Travis OSX artifacts 
#

# As of travis-ci-macos-10.13-xcode9.4.1-1529955246, the travis osx image
# contains a broken homebrew. Walk-around by reinstalling:
curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/uninstall > uninstall
chmod 755 uninstall
./uninstall -f
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" 

# bailout on errors and echo commands
set -xe
set -o pipefail

for pkg in cairo cmake libarchive libexif python3 wget xz; do
    brew list $pkg 2>/dev/null | head -10 || brew install $pkg
done
curl --version >/dev/null || brew install curl
wget http://opencpn.navnux.org/build_deps/wx312_opencpn50_macos109.tar.xz
tar xJf wx312_opencpn50_macos109.tar.xz -C /tmp
export PATH="/usr/local/opt/gettext/bin:$PATH"
echo 'export PATH="/usr/local/opt/gettext/bin:$PATH"' >> ~/.bash_profile

mkdir  build
cd build
test -z "$TRAVIS_TAG" && CI_BUILD=OFF || CI_BUILD=ON
cmake -DOCPN_CI_BUILD=$CI_BUILD \
  -DOCPN_USE_LIBCPP=ON \
  -DwxWidgets_CONFIG_EXECUTABLE=/tmp/wx312_opencpn50_macos109/bin/wx-config \
  -DwxWidgets_CONFIG_OPTIONS="--prefix=/tmp/wx312_opencpn50_macos109" \
  -DCMAKE_INSTALL_PREFIX=/tmp/opencpn -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 \
  ..
make -sj2
make package

make install
wget http://opencpn.navnux.org/build_deps/Packages.dmg;
hdiutil attach Packages.dmg;
sudo installer -pkg "/Volumes/Packages 1.2.5/Install Packages.pkg" -target "/"
make create-pkg

ls -l *.pkg *.tar.gz *.xml

# As of travis-ci-macos-10.13-xcode9.4.1-1529955246, the travis osx image
# contains a broken ruby setup used by deployment code. Walk around by
# reinstalling:
rm -rf /usr/local/lib/ruby/gems/ && brew reinstall ruby
