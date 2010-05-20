package com.androidemu;

import android.util.Log;

public class Emulator {

	public static final int VIDEO_W = 256;
	public static final int VIDEO_H = 240;
	public static final int GAMEPAD_TR		= (1 << 4);
	public static final int GAMEPAD_TL		= (1 << 5);
	public static final int GAMEPAD_X		= (1 << 6);
	public static final int GAMEPAD_A		= (1 << 7);
	public static final int GAMEPAD_RIGHT	= (1 << 8);
	public static final int GAMEPAD_LEFT	= (1 << 9);
	public static final int GAMEPAD_DOWN	= (1 << 10);
	public static final int GAMEPAD_UP		= (1 << 11);
	public static final int GAMEPAD_START	= (1 << 12);
	public static final int GAMEPAD_SELECT	= (1 << 13);
	public static final int GAMEPAD_Y		= (1 << 14);
	public static final int GAMEPAD_B		= (1 << 15);

	public static final int GAMEPAD_UP_LEFT = (GAMEPAD_UP | GAMEPAD_LEFT);
	public static final int GAMEPAD_UP_RIGHT = (GAMEPAD_UP | GAMEPAD_RIGHT);
	public static final int GAMEPAD_DOWN_LEFT = (GAMEPAD_DOWN | GAMEPAD_LEFT);
	public static final int GAMEPAD_DOWN_RIGHT = (GAMEPAD_DOWN | GAMEPAD_RIGHT);

	public native void setRenderSurface(EmulatorView surface,
			int width, int height);
	public native void setKeyStates(int states);
	public native void setOption(String name, String value);

	public native boolean initialize(String libdir, String engine);
	public native void cleanUp();
	public native void reset();
	public native void power();
	public native boolean loadROM(String file);
	public native void unloadROM();
	public native void pause();
	public native void resume();
	public native void run();
	public native boolean saveState(String file);
	public native boolean loadState(String file);

	public void setOption(String name, boolean value) {
		setOption(name, value ? "true" : "false");
	}


	static {

		System.loadLibrary("emu");

	}
}

