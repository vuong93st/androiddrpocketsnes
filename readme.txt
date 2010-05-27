Build environment requirements:
android-ndk to build the ARM bits, now using version r4
android-sdk - at least the 1.5 os bits
android source for includes
eclipse (or ant) to build the java bits

Where to unzip the source:
With the advent of r4 of the NDK we're no longer tied into the NDK folder. Phew.
Place the code wherever.  In fact, Application.mk in this directory is no longer
used.  You'll need to update project/jni/Android.mk, project/sound/Android.mk, and project/sneslib/Android.mk to reflect the correct folder structures 

About the Android.mk and Application.mk files:

For reference check out http://developer.android.com/sdk/ndk/index.html -- the 
android ndk comes with docs describing the .mk files.
As mentioned above, you need to modify each Android.mk file to correctly account
for the paths.  There's probably an easier methodology but I have yet to figure 
it out.  Make sure to update all the .mk file paths to point to your android 
source.  I found it easiest for portability to create symlinks (ln -s is your
friend).

Shared Libraries:

First, set up the bin folder for your build.  The folder 

project/bin/ndk/armeabi

needs to exist inside the source so you can copy some shared libs there.

The build will require shared libraries.  The following are needed:
	libdl.so
	libz.so
	libutils.so
	libnativehelper.so
	libmedia.so
	libunz.a

The .so files can all be found in the android source under the folder 
	/out/target/product/generic/system/lib/

And the linunz.a file can be found in 
	./out/host/linux-x86/obj/STATIC_LIBRARIES/libunz_intermediates/

Building the static android libs:

Inside the source tree, cd into project and run 'ndk-build'.  

If you have errors, consult the NDK documentation.

Building the APK:
In eclipse, import the project folder (where AndroidManifest.xml lives).  
Then just build the app.  It will see the built libraries and include them in
the APK file.  The APK will show up under the project folder as
bin/DrPocketSNES.apk.  You can verify the libraries are included by unzipping
the apk and looking for lib/armeabi/libemu.so libemusound.so and libsnes.so.
If they're missing, doublecheck that they exist in project/libs/armeabi.  If 
they're not there eclipse can't import them into the app.  Verify the ndk build
worked.  You can always debug with 'V=1 ndk-build -B' to rebuild and be verbose
about it.

Credits:
Credit goes to the snes9x team, the DrPocketSNES team, PocketSNES and SquidgeSNES teams. Some credit goes to Zhong Yang for creating the jni wrapper pieces.

License:
This source is being distributed under GPLv3, but original source for snes9x has its own license.  Refer to http://snes9x.com for more information.
