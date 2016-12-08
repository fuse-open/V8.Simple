set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set GYP_MSVS_VERSION=2013
set GYP_DEFINES="-Dv8_enable_i18n_support=0"
fetch v8
cd v8
git checkout remotes/branch-heads/5.5
gclient sync
python build\gyp_v8 -Dcomponent=static_library -Dv8_enable_i18n_support=0 -Dv8_use_snapshot=0 -Dtarget_arch=x64
# Open in VS2013
