Build environment requirements:
android-ndk to build the ARM bits
android-sdk - at least the 1.5 os bits
android source for includes
eclipse (or ant) to build the java bits

Where to unzip the source:
In the android NDK there will be an apps folder.  My suggestion is to place the source there as you can utilize the NDK make macros.

About the Android.mk and Application.mk files:

For reference check out http://developer.android.com/sdk/ndk/index.html -- the android ndk comes with docs describing the .mk files.
You'll need to tweak the Application.mk file to reflect the correct path to the source.  Also, you'll need to modify each Android.mk file to correctly account for the paths.  There's probably an easier methodology but I didn't figure it out.  Make sure to update all the .mk file paths to point to your android source.

Shared Libraries:

The build will require shared libraries.  The following are needed:
	libdl.so
	libunz.a
	libz.so
	libutils.so
	libnativehelper.so
	libmedia.so

In the android ndk there will be an out/apps/AndroidDrPocketSNES/armeabi folder.  It probably isn't created until the first make attempt, but the libraries all have to be copied there or you'll get errors in the build.

Building the android libs:
In the main android ndk folder (in my case /home/jrmuir/android-nkd-r3) run:
	make APP=AndroidDrPocketSNES

If you have errors, consult the NDK documentation.

Building the APK:
In eclipse, import the project from within the NDK folder.  Build the app.  It will see the built libraries and include them in the APK file.  The APK will show up under the project folder as bin/DrPocketSNES.apk.  You can verify the libraries are included by unzipping the apk and looking for lib/armeabi/libemu.so libemusound.so and libsnes.so.

Credits:
Credit goes to the snes9x team, the DrPocketSNES team, PocketSNES and SquidgeSNES teams. Some credit goes to Zhong Yang for creating the jni wrapper pieces.
