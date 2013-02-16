# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
FORMS += mainwindow.ui

# libssl android
#android {
LIBS += ../android-libs/openssl/lib/libssl.so ../android-libs/openssl/lib/libcrypto.so
INCLUDEPATH += ../android-libs/openssl/include
QMAKE_POST_LINK += \
    $${QMAKE_COPY} ../android-libs/openssl/lib/libssl.so $$PWD/android/libs/armeabi-v7a;
QMAKE_POST_LINK += \
    $${QMAKE_COPY} ../android-libs/openssl/lib/libcrypto.so $$PWD/android/libs/armeabi-v7a;
#}

# libssh android
#android {
LIBS += ../android-libs/libssh/lib/libssh.so
INCLUDEPATH += ../android-libs/libssh/include
QMAKE_POST_LINK += \
    $${QMAKE_COPY} ../android-libs/libssh/lib/libssh.so $$PWD/android/libs/armeabi-v7a;
#}

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    android/AndroidManifest.xml \
    android/res/drawable/icon.png \
    android/res/drawable/logo.png \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/version.xml \
    ../android-libs/openssl/lib/libssl.so \
    ../android-libs/openssl/lib/libcrypto.so \
    ../android-libs/libssh/lib/libssh.so
