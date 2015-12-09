export CXX=`which clang++`
export CC=`which clang`
export CPP="`which clang` -E -std=c++11 -stdlib=libc++"
export LINK="`which clang++` -std=c++11 -stdlib=libc++"
export CXX_host=`which clang++`
export CC_host=`which clang`
export CPP_host="`which clang` -E"
export LINK_host=`which clang++`
export GYP_DEFINES="clang=1 mac_deployment_target=10.9"
# make i18nsupport=off ia32.release
