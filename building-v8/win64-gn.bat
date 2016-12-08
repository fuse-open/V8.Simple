set DEPOT_TOOLS_WIN_TOOLCHAIN=0

set GYP_MSVS_VERSION=2013

set GYP_DEFINES="-Dv8_enable_i18n_support=0"

set GYP_GENERATORS=msvs
# fetch v8

cd v8

call git checkout remotes/branch-heads/5.5

call gclient sync

call gn gen out.gn/x64.release -vv --fail-on-unused-args --args="target_cpu=\"x64\" v8_current_cpu=\"x64\" v8_target_cpu=\"x64\" is_debug=false is_official_build=true symbol_level=1 v8_enable_i18n_support=false" --ide=vs