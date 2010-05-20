#define LOG_TAG "libsnes"
#include <utils/Log.h>
#include "snes9x.h"
#include "memmap.h"
#include "apu.h"
#include "soundux.h"
#include "snapshot.h"
#include "emuengine.h"
#include "file.h"
#include <unistd.h>

#define SCREEN_W		256
#define SCREEN_H		240
#define SCREEN_PITCH	(SCREEN_W * 2)

static EmuEngine::Callbacks *callbacks;

class SNesEngine : public EmuEngine {
public:
	SNesEngine();
	virtual ~SNesEngine();

	virtual bool initialize(Callbacks *cbs);
	virtual void destroy();
	virtual void reset();
	virtual void power();
	virtual Game *loadRom(const char *file);
	virtual void unloadRom();
	virtual bool saveState(const char *file);
	virtual bool loadState(const char *file);
	virtual void runFrame(bool skip);
	virtual void setOption(const char *name, const char *value);

private:
	bool apuEnabled;
	bool soundEnabled;
};


SNesEngine::SNesEngine()
		: apuEnabled(false),
		  soundEnabled(false)
{
}

SNesEngine::~SNesEngine()
{
	if (GFX.Screen) {
		free(GFX.Screen);
		GFX.Screen = NULL;
	}
	if (GFX.SubScreen) {
		free(GFX.SubScreen);
		GFX.SubScreen = NULL;
	}
	if (GFX.ZBuffer) {
		free(GFX.ZBuffer);
		GFX.ZBuffer = NULL;
	}
	if (GFX.SubZBuffer) {
		free(GFX.SubZBuffer);
		GFX.SubZBuffer = NULL;
	}

	fileCleanup();
}

bool SNesEngine::initialize(EmuEngine::Callbacks *cbs)
{
	callbacks = cbs;

	// Initialise Snes stuff
	memset(&Settings, 0, sizeof(Settings));

	Settings.JoystickEnabled = FALSE;
	Settings.SoundPlaybackRate = 22050;
	Settings.Stereo = FALSE;
	Settings.SoundBufferSize = 0;
	Settings.CyclesPercentage = 100;
	Settings.DisableSoundEcho = FALSE;
	Settings.APUEnabled = FALSE;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.Shutdown = Settings.ShutdownMaster = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	Settings.DisableSampleCaching = FALSE;
	Settings.DisableMasterVolume = FALSE;
	Settings.Mouse = FALSE;
	Settings.SuperScope = FALSE;
	Settings.MultiPlayer5 = FALSE;
	//	Settings.ControllerOption = SNES_MULTIPLAYER5;
	Settings.ControllerOption = 0;
	
	Settings.ForceTransparency = FALSE;
	Settings.Transparency = FALSE;
	Settings.SixteenBit = TRUE;
	
	Settings.SupportHiRes = FALSE;
	Settings.NetPlay = FALSE;
	Settings.ServerName [0] = 0;
	Settings.AutoSaveDelay = 30;
	Settings.ApplyCheats = FALSE;
	Settings.TurboMode = FALSE;
	Settings.TurboSkipFrames = 15;
	Settings.ThreadSound = FALSE;
	Settings.SoundSync = FALSE;
	//Settings.NoPatch = true;		

	if (Settings.ForceNoTransparency)
		Settings.Transparency = FALSE;
	if (Settings.Transparency)
		Settings.SixteenBit = TRUE;
	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

	GFX.Pitch = SCREEN_PITCH;
	GFX.Screen = (uint8 *) malloc(GFX.Pitch * SCREEN_H);
	GFX.SubScreen = (uint8 *) malloc(GFX.Pitch * SCREEN_H);
	GFX.ZBuffer = (uint8 *) malloc(SCREEN_W * SCREEN_H);
	GFX.SubZBuffer = (uint8 *) malloc(SCREEN_W * SCREEN_H);

	if (!Memory.Init() || !S9xInitAPU())
		return false;

	S9xInitSound();
	if (!S9xGraphicsInit())
		return false;

	Settings.SixteenBitSound = TRUE;
	so.stereo = Settings.Stereo;

	fileInitialize();
	return true;
}

void SNesEngine::destroy()
{
	delete this;
}

void SNesEngine::reset()
{
	Settings.APUEnabled = apuEnabled;
	S9xReset();
}

void SNesEngine::power()
{
	reset();
}

SNesEngine::Game *SNesEngine::loadRom(const char *file)
{
	Settings.APUEnabled = apuEnabled;
	if (!Memory.LoadROM(file))
		return NULL;

//	S9xLoadSRAM();

	Settings.ForceNTSC =
	Settings.ForcePAL = FALSE;
	Settings.PAL = (ROM[Memory.HiROM ? 0xffd9 : 0x7fd9] >= 2);

	Settings.FrameTime = (Settings.PAL ?
		Settings.FrameTimePAL : Settings.FrameTimeNTSC);
	Memory.ROMFramesPerSecond = (Settings.PAL ? 50 : 60);

	memset(GFX.Screen, 0, SCREEN_PITCH * SCREEN_H);

	static Game game;
	game.soundRate = Settings.SoundPlaybackRate;
	game.soundBits = 16;
	game.soundChannels = 1;
	game.fps = Memory.ROMFramesPerSecond;
	return &game;
}

void SNesEngine::unloadRom()
{
}

bool SNesEngine::saveState(const char *file)
{
	bool8 rv = S9xFreezeGame(file);
	sync();
	return (rv == TRUE);
}

bool SNesEngine::loadState(const char *file)
{
	if (!S9xUnfreezeGame(file))
		return false;

	if (!apuEnabled)
		CPU.APU_APUExecuting = Settings.APUEnabled = FALSE;
	return true;
}

void SNesEngine::runFrame(bool skip)
{
	IPPU.RenderThisFrame = (skip ? FALSE : TRUE);
	S9xMainLoop();

	if (Settings.APUEnabled && soundEnabled && !so.mute_sound) {
		short buffer[1024] __attribute__ ((aligned(4)));
		const unsigned int frameLimit = (Settings.PAL ? 50 : 60);
		const int sampleCount = Settings.SoundPlaybackRate / frameLimit;

		S9xMixSamples(buffer, sampleCount);
		callbacks->playAudio(buffer, sampleCount * 2);
	}
}

void SNesEngine::setOption(const char *name, const char *value)
{
	if (strcmp(name, "soundEnabled") == 0) {
		soundEnabled = (strcmp(value, "false") != 0);
		S9xSetPlaybackRate(soundEnabled ? Settings.SoundPlaybackRate : 0);

	} else if (strcmp(name, "apuEnabled") == 0) {
		apuEnabled = (strcmp(value, "false") != 0);
		if (!apuEnabled)
			CPU.APU_APUExecuting = Settings.APUEnabled = FALSE;

	} else if (strcmp(name, "transparencyEnabled") == 0) {
		Settings.Transparency = (strcmp(value, "false") != 0);
	}
}


extern "C"
void S9xSetPalette()
{
}

extern "C"
bool8_32 S9xInitUpdate()
{
	return TRUE;
}

extern "C"
bool8_32 S9xDeinitUpdate(int width, int height, bool8_32)
{
	EmuEngine::Surface surface;
	if (!callbacks->lockSurface(&surface))
		return FALSE;

	if (surface.bpr == SCREEN_PITCH)
		memcpy(surface.bits, GFX.Screen, SCREEN_H * SCREEN_PITCH);
	else {
		uint8 *d = (uint8 *) surface.bits;
		uint8 *s = GFX.Screen;
		for (int h = SCREEN_H; --h >= 0; ) {
			memcpy(d, s, SCREEN_PITCH);
			d += surface.bpr;
			s += SCREEN_PITCH;
		}
	}
	callbacks->unlockSurface(&surface);
	return TRUE;
}

extern "C"
uint32 S9xReadJoypad(int which)
{
	uint32 states = 0x80000000;
	if (which == 0)
		states |= callbacks->getKeyStates();
	return states;
}

extern "C"
bool8_32 S9xReadMousePosition(int which1_0_to_1,
		int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

extern "C"
bool8_32 S9xReadSuperScopePosition(int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

extern "C" __attribute__((visibility("default")))
EmuEngine *createEngine()
{
	return new SNesEngine;
}
