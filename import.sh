SOURCE_TOP_DIR=/james/Code/APP
CURRENT_DIR=`pwd`

unset ez_LinkDir
ez_LinkDir=( \
    "build" \
    "cts" \
    "bionic/libc"\
    "System/"\
    "vendor/tct/source/qcn" \
    "vendor/tct/source/apps/PhoneCommon" \
    "vendor/tct/source/apps/JrdWFCManager" \
    #"development/apps/MMITest/" \
    "libcore/luni" \
    "libcore/libart" \
    "libcore/dalvik/" \
    "device/qcom/common" \
    "device/tct" \
    #"vendor/tct/source/system" \
    "frameworks/base" \
    "frameworks" \
    "vendor/tct/source/apps/Dialer"\
    "vendor/tct/source/apps/InCallUI"\
    "vendor/tct/source/apps/ContactsCommon"\
    "vendor/tct/source/apps/Mms"\
    "vendor/"\
    "hardware"\
    "amss_8996"\
    "vendor/tct/source/apps/" \
    "frameworks/opt/net/ims" \
    "frameworks/opt/net/wifi" \
    "frameworks/opt/telephony" \
    "packages/apps/Settings" \
    "packages/services/Telecomm" \
    "packages/services/Telephony" \
    "packages/apps/Stk" \
    "packages/apps/PhoneCommon" \
    "packages/providers/TelephonyProvider" \
    "packages/providers/ContactsProvider" \
    "packages/apps/PackageInstaller" \
    "system/core" \
    "external/junit/src/junit/framework" \
    "libcore/xml/src" \
#    "packages/apps/Bluetooth" \
#    "amss_8976" \
    "vendor/qcom/proprietary" \
#    "vendor/tct/source/apps/TctFeedback" \
    #"vendor/tct/source/apps/JrdWFCManager" \
    #"vendor/tct/source/apps/TctPowerRollbackServices" \
    #"vendor/tct/source/qcn/auto_make_tar" \
#    "out/target/common/R/com/jrdcom" \
    "out/target/common/obj/JAVA_LIBRARIES/framework_intermediates" \
    "out/target/common/R/com/android/phone" \
    "out/target/common/obj/APPS/ims_intermediates" \
    "out/target/common/R/com/android/internal" \
    out/target/common/obj/APPS/framework-res_intermediates/src \
#    "amss/modem_proc/mcfg/mcfg_gen/scripts/data/efs_files/tmo" \
#    "out/target/common/obj/APPS/JrdWFCManagerTest_intermediates" \
#     "out/target/common/obj/APPS/JrdWFCManager_intermediates" \
#     "out/target/common/obj/APPS/SmartcardService_intermediates" \
    "packages/apps/InCallUI" \

)

# check source dir
if [ ! -e "$SOURCE_TOP_DIR" ]; then
    echo -e "\e[31mPlease input the top dir of your source.\e[0m"
    exit
fi

for i in "${!ez_LinkDir[@]}"; do
    single=`echo ${ez_LinkDir[$i]} | grep /`
    if [ -z $single ]; then
        dir=
    else
        dir=${ez_LinkDir[$i]%/*}
    fi
    name=${ez_LinkDir[$i]##*/}

    # check if link target existed
    if [ ! -e $SOURCE_TOP_DIR/$dir/$name ]; then
        echo -ne "\e[30m"
        echo -n "\"$SOURCE_TOP_DIR/$dir/$name\" Not existed, ignore this dir."
        echo -e "\e[0m"
        continue
    fi

    # skip if link target has already linked
    if [ -e $CURRENT_DIR/$dir/$name ]; then
        echo -ne "\e[30m"
        echo -n "\"$dir/$name\" existed, skipped."
        echo -e "\e[0m"
        continue
    fi

    # make a dir for link target
    if [ ! -e $dir ]; then
        echo -ne "\e[33m"
        echo -n "mkdir -p $dir"
        echo -e "\e[0m"
        mkdir -p $dir
    fi

    # link target here
    path=$dir/$name
    if [ -z $dir ]; then
        path=$name
    fi
    echo -ne "\e[33m"
    echo -n "ln -s $SOURCE_TOP_DIR/$path $path"
    echo -e "\e[0m"

    ln -s $SOURCE_TOP_DIR/$path $path
done
