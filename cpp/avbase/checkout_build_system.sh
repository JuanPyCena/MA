#!/bin/bash

# This template file is copied into every package and is used by the package build script to
# retrieve the correct build system.
#
# *ATTENTION* If you need to change this file, ensure that the template is updated!

# environment variable interface:
# bamboo_override_list

THIS_SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]:-$0}" )" && pwd )

BUILD_SYSTEM_GIT_URL=ssh://git@bitbucket.avibit.com:7999/avbs/build-system.git

GIT_NUMERIC_VERSION_TAG_PREFIX='v'
VERSION_RE="^[0-9]+\.[0-9]+\.[0-9]+(-[a-z]+[^-]*[1-9][0-9]*)?$"

if [ -d "build-system" ]; then
    echo "Info: existing build-system will be removed."
    rm -rf build-system
fi

bs_version=$(cat ${THIS_SCRIPT_DIR}/required_packages.cmake | grep "av_find_avibit_package(build-system" | awk '{if(NF > 2) print $2; else print substr($2, 1, length($2)-1)}')

for override_parameter in echo $bamboo_override_list
do
    arrIN=(${override_parameter//:/ })
    if [[ ${arrIN[0]} == "build-system" ]]; then
       bs_version=${arrIN[1]}
       echo "build-system override parameter found!"
    fi
done

if [[ $bs_version =~ $VERSION_RE ]]; then
   bs_version=${GIT_NUMERIC_VERSION_TAG_PREFIX}${bs_version}
fi

echo "build-system branch/tag used: ${bs_version}"
git clone $BUILD_SYSTEM_GIT_URL -b $bs_version

