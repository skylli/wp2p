set PATH=D:\Android\android-ndk-r8
set PATH=%PATH%;%PATH%\toolchains\arm-linux-androideabi-4.4.3\prebuilt\windows\bin
arm-linux-androideabi-ar.exe -x libSmartConnection.a
copy obj\local\armeabi\libulink.a libulink.a
arm-linux-androideabi-ar.exe -qf libulink.a broadcastSSID.o crypt_aes.o crypt_hmac.o crypt_sha2.o SmartConnection.o
copy libulink.a ..\jni
