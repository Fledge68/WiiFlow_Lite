_____________________________________________________________________________
|0O×øo·                         Snes9x GX                             ·oø×O0|
|                   http://code.google.com/p/snes9x-gx                      |
|                          (Under GPL License)                              |
|___________________________________________________________________________|

Snes9x GX is a Super Nintendo™ / Super Famicom emulator for the Nintendo Wii.
Snes9x GX is a port of Snes9x (http://www.snes9x.com).

Snes9x GX is a "homebrew application" which means you will need a way to run
unsigned code on your Nintendo Wii. The best website for getting started with
Wii homebrew is WiiBrew (www.wiibrew.org).

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                                FEATURES  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

* Based on Snes9x 1.52
* Wiimote, Nunchuk, Classic, and Gamecube controller support
* SNES Superscope, Mouse, Justifier support
* Cheat support
* Auto Load/Save Game Snapshots and SRAM
* Custom controller configurations
* SD, USB, DVD, SMB, Zip, and 7z support
* Autodetect PAL/NTSC, 16:9 widescreen support
* Original/filtered/unfiltered video modes
* Turbo Mode - up to 2x the normal speed
* Zoom option to zoom in/out
* Open Source!

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                          UPDATE HISTORY  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

[4.3.4 - February 2, 2016]

* Fixed screenshot image dimensions
* Fixed screenshot option, it no longer creates an additional "dummy" file
* Fixed performance issues, reverted to previous vsync method
* Compiled with devkitPPC r26
  Note: If compiled with devkitPPC r27 some games, like Final Fantasy III
	and Chrono Trigger would crash on a battle

[4.3.3 - September 3, 2015July 7, 2015]

* Added Cebolleto's preview image support
* Added WiiUPro controller support
* Added a Scanline filter option
* Increase preview image size and reduce game list width
* Added a background to the preview image
* Added a Screenshot button
* Added a "WiiuPro" button on the button mapping menu
* Fixed the inverted color button selection that was in some option windows

[4.3.2 - November 9, 2012]

* Fixed lag with GameCube controllers

[4.3.1 - July 7, 2012]

* Fixed PAL support

[4.3.0 - July 6, 2012]

* Support for newer Wiimotes
* Fixed screen flicker when going back to menu
* Improved L/R buttons for GameCube - trigger button press when 50%+ depressed
* Improved controller behavior - allow two directions to be pressed simultaneously
* Timing changes
* Compiled with devkitPPC r26 and libogc 1.8.11

[4.2.9 - January 14, 2012]

* More accurate pixel scaling (thanks eke-eke!)
* Other minor changes

[4.2.8 - May 15, 2011]

* Ported a few upstream game fixes for: Mutant Chronicles - Doom Troopers, 
  Porky Pig's Haunted Holiday, Big Sky Trooper, and Mechwarrior 3050 / Battle 
  Tech 3050
* Added Turkish translation

[4.2.7 - March 23, 2011]

* Fixed browser regressions with stability and speed
* Reverted zones' changes to IRQ and SA-1 - these caused a performance hit

[4.2.6 - March 19, 2011]

* Improved game compatibility (updated core to latest Snes9x)
* Fixed issue with cheats not persisting
* Improved USB and controller compatibility (recompiled with latest libogc)
* Enabled SMB on GameCube (thanks Extrems!)
* Added Catalan translation
* Translation updates

[4.2.5 - October 7, 2010]

* Sync with upstream SVN - fixes a few specific game issues
* Fixed "blank listing" issue for SMB
* Improved USB compatibility and speed
* Added Portuguese and Brazilian Portuguese translations
* Other minor changes

[4.2.4 - August 14, 2010]

* IOS 202 support removed
* USB 2.0 support via IOS 58 added - requires that IOS58 be pre-installed
* DVD support via AHBPROT - requires latest HBC

[4.2.3 - July 22, 2010]

* Fixed broken auto-update

[4.2.2 - July 20, 2010]

* Reverted USB2 changes
* Fixed return-to-menu crash with Filtered mode

[4.2.1 - July 14, 2010]

* Improved audio timing
* Fixed 16:9 correction in Original mode
* Ability to use both USB ports (requires updated IOS 202 - WARNING: older 
  versions of IOS 202 are NO LONGER supported)
* Fixed issue with IPS patch files
* Hide non-ROM files
* Other minor improvements

[4.2.0 - June 20, 2010]

* Fixed incorrect aspect ratio issue
* Fixed choppy framerate issue
* Fixed IPS/UPS patching
* USB improvements
* GameCube improvements - audio, SD Gecko, show thumbnails for saves
* Other minor changes

[4.1.9 - May 19, 2010]

* DVD support fixed
* Fixed some cheats issues
* Fixed some potential hangs when returning to menu
* Video/audio code changes
* Fixed scrolling text bug
* Other minor changes

[4.1.8 - April 9, 2010]

* Fix auto-save bug

[4.1.7 - April 9, 2010]

* Freezing issue fixed
* Core timing fixes, should solve some problems with some games
* Most 3rd party controllers should work now (you're welcome!)
* 7z crash fixed
* GameCube only - DVD/SD Gecko fixed (thanks emukidid!)
* Translation updates (German and Dutch)
* Other minor changes

[4.1.6 - March 30, 2010]

* New core! Updated to Snes9x 1.52. Old save states will not work!
* DVD / USB 2.0 support via IOS 202. DVDx support has been dropped. It is
  highly recommended to install IOS 202 via the included installer
* Multi-language support (only French translation is fully complete)
* Thank you to everyone who submitted translations
* SMB improvements/bug fixes
* Minor video & input performance optimizations
* Now uses .srm files without "Auto" appended, if found
* Added option to disable crosshair

[4.1.5 - December 23, 2009]

* Changed default folders to snes9xgx/roms and snes9xgx/saves. Old folders
  will be automatically moved to the new location
* File browser now scrolls down to the last game when returning to browser
* Auto update for those using USB now works
* Fixed scrollbar up/down buttons
* Minor optimizations

[4.1.4 - December 2, 2009]

* Fixed SMB (for real this time!)
* Fixed snapshot loading regression

[4.1.3 - November 30, 2009]

* Fixed SMB
* Added separate horizontal/vertical zoom options
* Improved scrolling timing - the more you scroll, the fast it goes
* Fixed reset button on Wii console - now you can reset multiple times
* Minor code optimizations
* Reduce memory fragmentation - fixes out of memory crashes

[4.1.2 - October 7, 2009]

* Revamped filebrowser and file I/O
* Many, many other bug fixes

[4.1.1 - September 19, 2009]

* Fixed black screen bug

[4.1.0 - September 16, 2009]

* Text rendering corrections
* SMB improvements
* Built with latest libraries
* Video mode switching now works properly
* Other minor bugfixes and cleanup

[4.0.9 - July 22, 2009]

* Fixed "No game saves found." message when there are actually saves.
* Fixed shift key on keyboard
* Text scrolling works again
* Change default prompt window selection to "Cancel" button

[4.0.8 - July 9, 2009]

* Faster SMB/USB browsing
* Last browsed folder is now remembered
* Fixed controller mapping reset button
* Fixed no sound on GameCube version
* Directory names are no longer altered
* Preferences now only saved on exit
* Fixed on-screen keyboard glitches
* SRAM auto-saved on power-off from within a game
* Prevent 7z lockups, better 7z error messages
* Increase cheats limit from 30 to 150
* Cheats no longer effect next game loaded - they are cleared properly

[4.0.7 - June 30, 2009]

* Fixed auto-update
* Increased file browser listing to 10 entries, decreased font size
* Added text scrolling on file browser
* Added reset button for controller mappings
* Settings are now loaded from USB when loading the app from USB on HBC
* Fixed menu crashes caused by ogg player bugs
* Fixed memory card saving verification bug
* Fixed game savebrowser bugs
* Miscellaneous code cleanup/corrections

[4.0.6 - May 30, 2009]

* Fixed SD/USB corruption bug
* SMB works again
* GUI bugs fixed, GUI behavioral improvements

[4.0.5 - May 26, 2009]

* Improved stability
* Fixed broken SDHC from HBC 1.0.2 update
* Fixed issues with returning to menu from in-game
* Add option to disable rumble
* Auto-determines if HBC is present - returns to Wii menu otherwise
* Fixed Super Scope turbo button issue
* Miscellaneous bugfixes

[4.0.4 - April 30, 2009]

* Improved scrollbar
* Built with more stable libogc/libfat
* Fixed rumble bug in filebrowser
* Added confirmation prompts
* Fixed settings saving glitches

[4.0.3 - April 23, 2009]

* Settings resetting glitch fixed

[4.0.2 - April 22, 2009]

* GameCube controller home trigger fixed
* USB support fixed
* More stable SMB support
* Corrections/improvements to game saving/loading
* Video mode corrections
* hq2x filter no longer crashes hi-res games on unfiltered/filtered modes
* Corrections to cheat file parsing
* Super Scope turbo now activated when holding Down button
* Settings are now saved when exiting game menu settings area
* Super Scope pause button now works
* Invalid VRAM access is now allowed - should fix some homebrew games
* New video mode selection in menu (forcing a video mode is not recommended)

[4.0.1 - April 13, 2009]

* Fixed turning off autosave in menu
* Keyboard fixed, more keys added
* Fixed 7z loading from DVD
* Enable auto loading and renaming of SRAM save without "Auto" in the name
* PAL resolution fixed - no more letterboxing
* File browser scrollbar box now draggable, arrow buttons now scroll list
* Better handling of multiple on-screen wiimotes
* Menu restructured - new "Game Settings" menu

[4.0.0 - April 4, 2009]

* New GX-based menu, with a completely redesigned layout. Has Wiimote IR 
  support, sounds, graphics, animation effects, and more
* Thanks to the3seashells for designing some top-notch artwork, to 
  Peter de Man for composing the music, and a special thanks to shagkur for 
  fixing libogc bugs that would have otherwise prevented the release
* Onscreen keyboard for changing save/load folders and network settings
* Menu configuration options (configurable exit button, wiimote orientation,
  volumes)
* Configurable button mapping for superscope, mouse and justifier
* New save manager, allowing multiple saves and save browsing. Shows
  screenshots for Snapshot saves, and save dates/times
* Experimental hq2x filter, contributed by michniewski

[009 - January 27, 2009]

* Faster SD/USB - new read-ahead cache
* Removed trigger of back to menu for Classic Controller right joystick
* Fixed a bug with reading files < 2048 bytes
* Fixed some memory leaks, buffer overflows, etc
* Code cleanup, other general bugfixes

[008 - December 24, 2008]

* Fixed unstable SD card access
* Proper SD/USB hotswap (Wii only)
* Auto-update feature (Wii only)
* Rewritten SMB access - speed boost, NTLM now supported (Wii only)
* Improved file access code
* Resetting preferences now resets controls
* Fixed "invalid file size" error on smaller games (eg: Space Invaders)
* Fixed hangup in Super Mario RPG
* Minor bug fixes

[007 - November 19, 2008]

* added: SDHC support
* added: SD/USB hot-swapping
* added: zoom saving
* added: IPS/UPS/PPF patch support
* added: Qoob modchip support (thanks emukidid!)
* added: Added console/remote power button support (Wii only)
* added: Added reset button support - resets game (Wii only)
* changed: Settings file is now named settings.xml and is stored in the same
           folder as the DOL - eg: apps/Snes9xGX/settings.xml (Wii only)
* fixed: swc, sfc file support
* fixed: inverted sound channels
* fixed: some game crashes
* fixed: snapshot saving for games with SPC7110, BS, DSP, or CX4
* fixed: justifier support
* fixed: superscope turbo button
* fixed: widescreen support
* fixed: ActRaiser 2 issues
* fixed: Invalid memory accesses in C4 and OBC1. MMX3 Toxic Seahorse stage
         now can be emulated with HDMA. (zones)
* fixed: Updated snapshot to prevent desync. Added some variables related to
         APU and HDMA. (gocha)
* fixed: zoom issues (thanks eke-eke!)
* fixed: original modes (thanks eke-eke!)
* fixed: vertical scaling for filtered/unfiltered modes (thanks eke-eke!)

[006 - October 15, 2008]

Tantric
* added: 480p for GameCube
* added: Sound sync
* added: 7z support
* changed: Faster SD/USB access (readahead cache enabled)
* fixed: Video offset issues
* fixed: BS-X games work now
* fixed: Wii DVD works now
* fixed: DVD re-enabled for GameCube
* fixed: Nunchuk analog stick issues
* fixed: Many crashes, memory leaks, etc

michniewski
* added: Video shift options
* changed: Turn DVD motor off option re-enabled for GameCube

[005 - September 23, 2008]

michniewski
* added: Superscope/mouse/justifier support, with Wii remote
* added: 3 render modes - Original, Filtered, Unfiltered
* added: widescreen compensation option
* added: DVD support on the Wii
* changed: zoom - limited range, added reset zoom option

Tantric
* added: now uses SNES 1.51 core (thanks to eke-eke for help with this)
* added: cheats menu! Loads .CHT file from /snes9x/cheats folder, 
         .CHT file name must match file name of ROM
* added: load/save preference selector. ROM, SRAM, Freeze, and preferences 
         are saved/loaded according to these
* added: preliminary Windows file share loading/saving (SMB) support on Wii:
		 You can input your network settings into snes9xGX.xml, or edit 
		 s9xconfig.cpp from the source code and compile.
* added: 'Auto' settings for save/load - attempts to automatically determine
         your load/save device(s) - SD, USB, Memory Card, DVD, SMB
* added: ROM Information page
* added: Game Menu - all game-specific options are here now: 
         SRAM save/load, Snapshot save/load, game reload, etc
* added: Credits page
* fixed: sd gecko works now
* fixed: full USB support
* changed: menu structure
* changed: preferences are now loaded and saved in XML format. You can open
		   snes9xGX.xml edit all settings, including some not available within
		   the program
* changed: if Home button is pressed when a game is running, Game Menu pops up
* changed: if preferences can't be loaded at the start and/or are reset, 
           preferences menu pops up - remove to save your preferences!
* changed: SRAM load - game reloaded automatically after loading SRAM

[004 - August 5, 2008]

* added: option to disable AA filtering 
         (snes graphics 'crisper', AA now default OFF)
* added: mapped zooming and turbo mode to classic controller
* added: preliminary usb support (loading)
* changed: sram and freezes now saved by filename, not internal romname. 
           If you have multiple versions of the same game, you can now have 
           srams and freezes for each version. A prompt to convert to the 
           new naming is provided for sram only.
* changed: by default, autoload/save sram and freeze enabled

[003 - July 25, 2008]
* added: alphabetical file sorting
* added: background logo/backdrop + nicer menus
* added: scrolling in ROM selector
* fixed: switching between pal/ntsc ROMS doesn't mess up timings
* fixed: GC controller config works now
* fixed: freeze autoloading on ROM load
* fixed: zipped ROMS should now load in a reasonable time
* fixed: precompiled dols for autosaving to various locations (see readme)
* changed: GC default quickload slot (to sd) (thanks kerframil)
* changed: default load/save dirs are now "/snes9x/roms" and 
           "/snes9x/saves/"  (thanks kerframil)
* changed: Classic X and Y defaults aren't switched
* changed: if autosave is enabled, it doesn't ask to save SRAM 
           anymore. It is saved in the background.
* updated README

[002 - July 21, 2008]
* added: classic and nunchuk support
* added: all controllers can now be configured
* added: GC version (untested)
* changed: mappings are no longer stored in SRAM, but in config file. 
           This means no per-game configurations, but one global 
           config per controller.
* one makefile to make all versions. (thanks to snes9x143 SVN)


[001 - July 12, 2008]
* compiles with latest devkitppc (r15)
* now uses libfat (can use front sd slot on wii)
* updated menu items a bit
* wiimote support
* fixed: autoload sram/freeze
* fixed: rom plays immediately after loading

[older update history at the bottom]

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                            INSTALLATION  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

To install Snes9x GX you will need to copy over the "apps" and "snes9xgx" 
folders (directories) from this archive to your SD Card. It comes 
pre-packaged in the Homebrew Channel format (which you will use to launch 
the emulator). Once you've copied the directories to your SD Card you will 
need to place your ROM image(s) into "\snes9xgx\roms" directory and place any
cheat code files (optional) in the "\snes9xgx\cheats" directory. Once this is
done you can proceed to run the emulator via the Homebrew Channel.

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                           INITIAL SETUP  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

The first time you run Snes9x GX it will create a new "settings.xml" file on
the SD Card which stores the configuration of the emulator. So when you run
Snes9x GX it will automatically detect your storage medium and will bring you
to the ROM selection screen. If you want to run a game with the default 
settings then just highlight the game and press "A" on it and it will load. 
However, you can configure Snes9x GX to your liking.

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                           CONFIGURATION  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

To configure Snes9x GX press "A" on the "Settings" box. This will brings up 
the "Settings" screen where you can configure the Button Mappings, Video, 
Saving, Menu and Network. You can also restore the settings to their initial 
values by clicking on the "Reset Settings" and you can return to the ROM 
selection screen by clicking on "Go Back".

--•[ Button Mappings ]•--
	
Once in the Button Mappings settings menu you'll have the option to pick to
configure the SNES Controller, Super Scope, SNES Mouse and the Justifier.
Once you select a controller to configure you will be presented with which
input device you would like to configure. To set the button simply click on
the input device you wish to use and then pick the button you wish to map. 
After you select a button to configure Snes9x GX will prompt you to press
the button you wish to assign to the button you've selected. Below is a list
of each controller followed by the input devices and the default values for
those devices.
	
	[ SNES Controller ]
		
		• GameCube Controller
			
			A = A
			B = B
			X = X
			Y = Y
			L = L
			R = R
			START = START
			SELECT = Z
			UP = UP
			DOWN = DOWN
			LEFT = LEFT
			RIGHT = RIGHT
			
		• Wiimote
		
			A = B
			B = 2
			X = 1
			Y = A
			START = PLUS
			SELECT = MINUS
			UP = UP
			DOWN = DOWN
			LEFT = LEFT
			RIGHT = RIGHT
		
		• Classic Controller

			A = A
			B = B
			X = X
			Y = Y
			L = L
			R = R
			UP = UP
			DOWN = DOWN
			LEFT = LEFT
			RIGHT = RIGHT

		•	Wiimote Nunchuck + Wiimote
		
			A = A
			B = B
			X = C
			Y = Z
			L = 2
			R = 1
			START = PLUS
			SELECT = MINUS
			UP = UP
			DOWN = DOWN
			LEFT = LEFT
			RIGHT = RIGHT
		
	[ Super Scope ]

		• GameCube Controller

			FIRE = A
			AIM OFFSCREEN = B
			CURSOR = Z
			TURBO ON = Y
			TURBO OFF = X
			PAUSE = START

		• Wiimote
		
			FIRE = B
			AIM OFFSCREEN = A
			CURSOR = MINUS
			TURBO ON = UP
			TURBO OFF = DOWN
			PAUSE = PLUS

	[ SNES Mouse ]
	
		• GameCube Controller
		
			LEFT BUTTON = A
			RIGHT BUTTON = B
		
		• Wiimote
		
			LEFT BUTTON = A
			RIGHT BUTTON = B
			
	[ Justifier ]
	
		• GameCube Controller
		
			FIRE = B
			AIM OFFSCREEN = A
			START = START
		
		• Wiimote

			FIRE = B
			AIM OFFSCREEN = A
			START = PLUS
		
--•[ VIDEO ]•--

The Video settings screen will allow you to configure various video settings
including the Rendering method, Scaling, Filtering, Screen Zoom and Screen	
Position. You can toggle the options for the first three selections by 
clicking on them. To make the Filtering option you select you must enable
Filtering under the Rendering setting.

• Rendering - Unfiltered
            - Original
            - Filtered
          
• Scaling 	- Default
				  	- 16:9 Correction

• Filtering - None
				  	- hq2x
					  - hq2x Soft
					  - hq2x Bold
					
Screen Zoom will bring up a menu where you can adjust the Zoom level by using
the left and right arrows that you can click on. The default setting is 100%.

The Screen Position option will bring up a screen with four arrows that 
allows you to move the position of the output. You'll only need to use this 
option if the screen is not centered. Combining this with the Zoom and
Scaling you should be able to get a proper display on your television.

--•[ SAVING & LOADING ]•--

From this setup option you can configure the way Snes9x GX loads and saves.
Your options are Load Method, Load Folder, Save Method, Save Folder, Auto
Load and Auto Save. Clicking on all of the options except the folders will
let you toggle through their settings. Clicking on the folder options will
bring up a text box where you can enter in the folder you wish to use for
your save data. Snes9x GX has two types of saves - one is the SRAM which is
the save game data (this is only applicable if the game you're playing 
supports saving) and the other is Snapshots which are real time saves. Real
time saves allow you to save your game in it's current state and resume it
at a later time.

	• Load Method -	SD, USB, DVD, Network and Auto
	
	• Save Method - SD, USB, Network and Auto

	• Auto Load - SRAM, Snapshot and Off

	• Auto Save - SRAM, Snapshot and Off

--•[ MENU ]•--

This settings screen will allow you to customize a few aspects on how the 
Menu works including the Exit Action, Wiimote Orientation, Music Volume and
the Sound Effects Volume. The Exit Action can be set to "Return to Loader",
"Return to Wii Menu" and "Power off Wii". The Wiimote Orientation can be set
to Vertical or Horizontal and then you can increase and decrease the Music
and Sound Effects Volume.

--•[ NETWORK ]•--

If you're going to be using your Network (LAN) to load and/or save from you
will need to enter in the SMB share settings you've setup on your computer
via this settings menu. You will need to enter in the SMB Share IP, Share
Name, Share Username and Share Password. If you do not have a password on
your SMB share then set the SMB Share Password to "0".

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                            FILE BROWSER  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

The File Browser will automatically load when you start the emulator and it
will automatically list the contents of the "\snes9xgx\roms\" directory on
your SD card. Like mentioned above in the Configuration -> Saving & Loading
you can specifically a different directory to load from. To launch a game 
(the game can either be uncompressed or inside a ZIP archive) simply click 
on it and it will load. You may also change the directory you're in by 
clicking on "Up One Level".

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                                GAMEPLAY  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

Once you load a game you can access another option menu by pressing the 
"Home" button. This will bring up the Save, Load, Reset, Controller and 
Cheats options. From this menu you can also return to the File Browser by
selecting "Main Menu". To leave the menu and resume game play select "Close".

The Save option will allow you to save either your SRAM or a Snapshot. Once
you've selected Save two options will appear named "New SRAM" and "New 
Snapshot". Clicking on these will create a new save. Once you have a save
you can save over it by selecting it instead of a "New" option. The Load
option will allow you to load your saved SRAM or Snapshot. Reset will reset
the game. If you pick Controller you can toggle between which controller you
wish to use to control your game with. The last option, Cheats, will allow you
to toggle your cheat codes (see below). 

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                                  CHEATS  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

Cheats are loaded from the "\snes9xgx\cheats" directory and must be in the
SNES9X .CHT file format. They also must be named the same name as the game
you're playing with the .cht extentsion. An example would be if you have a 
ROM called "Super Mario World.smc" you would need to name your .cht file to
"Super Mario World.cht". You can find a link to a massive collection of Super
NES cheat codes in the Links section (below).

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                              TURBO MODE  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

TurboMode increases the playback speed of the game by about 2x. To use 
TurboMode simply press the c-stick (yellow control stick) to the right and 
hold it right as long as you want playback to be double-speed. Release the 
c-stick when you want normal playback speed to resume.

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                            IMPORTING AND EXPORTING SRAM  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

Snes9x GX now includes the ability to load SRAM from Snes9x on other
platforms (Mac/PC/Linux/etc) and to save back to those platforms. 

To load a SRAM file on the Wii or Gamecube from another platform, ensure the
name of the SRM file matches the filename of the ROM (except with an SRM 
extension).

To use a Wii/GameCube SRAM file on another platform just do the opposite: 
copy the saved SRAM file to the other platform. You may have to rename the 
file to be what that version of snes9x expects it to be.

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                    UPDATE HISTORY (OLD)  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

[2.0.1b8]
* Added: SD slot B options for freezes, sram and loading of roms
* Changed: SMB options no longer displayed in menus when run on a Wii
* Changed: Game auto resumes running after resetting when choosing the "Reset
    Game" menu option
* Fixed (maybe): Reading of DVDs past the 1.36 GB barrier (Wii only) please
    test! - svpe

[2.0.1b7]
* Fixed: Zip compressed freezes to memory card could become corrupted as the
    file size changed - fixed this by deleting the existing file before writing
    a new one if the file size increased. If the file got smaller or stayed the
    same the original file is updated, using less of the existing file if the
    actual file size is smaller. A check is made before deleting the existing
    freeze file to ensure that there is enough space available for the new
    file. Note that additional space equivalent to the new file size must be
    available. If not enough space is available the original freeze is retained
    and the user is informed of the lack of space.
* Fixed: If option to auto-load freeze was selected, joypad config would not
    be restored since that is stored in SRAM. Resolved this for now by first
    loading SRAM if any and then loading the freeze. Obviously having to have
    both SRAM and freeze is not ideal, but this gets the job done if you have
    enough space on your memory card, SD card, etc.
* Added prompt when returning to the menu with autosave enabled allowing the
    user choose whether or not to perform the save. Press A to save or B if you
    don't wish to save.
* Added optional verification of Gamecube memory card saves. After writing
    the file it reads it back in and verifies that the written file matches
    what was to be saved. If it doesn't or if there was a problem opening the
    file it reports the problem to the user. Set this option in the preferences
    if desired.
* Added Reset Gamecube/Wii menu item
* Experimental DVD support for reading beyond 1.36 GB barrier on Wii. I have
    no way to test this, so please report on whether or not it works! Based on
    svpe's code.
    
NOTE: due to changes in the settings, this version will reset your emulator
options settings, so if you had saved preferences you will need to make your
changes to the emulator settings again and save them.

[2.0.1b6a]
* Fixed: Going up a directory when selecting a rom on a DVD wasn't working
    
[2.0.1b6]
* PAL Wii support - no separate version necessary! - eke-eke
* PAL roms now play at correct speed via internal timer, ntsc roms still use
    more accurate VSYNC timing - eke-eke
* Zipped freezes to memory card - take 9-12 blocks or so - based on denman's
    code
* Added option for auto save and load of freezes. For saving, can do both SRAM
    and Freeze if desired
* Memory card saving and loading shows a progress bar
* More miscellaneous ease-of-use improvements and cleanup
* Fixed: pressing B to get out of a rom file selection screen no longer drops
    you all the way back to the main menu. Now goes back to choice of where to
    load ROM (the "Load from DVD", "Load from SMB"... screen)
* Fixed: loading of joypad configuration in SRAM works again - no longer gets
    messed up

[2.0.1b5]
* B button implemented in all menus (returns to previous menu)
* Fixed bug when freezing state to SD card - would crash if SD support was not
    previously initialized
* Fixed double A button press needed after manual prefs/sram save to memory card
* Fixed delay after pressing A button after saving freeze to SD card
* Fixed problem of ".srm" SRAM file being created when SRAM was saved with no
    ROM loaded
* Fixed version number in SRAM and preferences
* Minor other code revisions

[2.0.1b1 through 2.0.1b4]
* SRAM saving and loading from snes9x on other platforms via SD card or SMB
* Games now autostart once loaded
* After manually loading SRAM the emulator is automatically reset
* Optional auto-loading of SRAM from memory card, SD or SMB after game loads
* Optional auto-saving of SRAM to memory card, SD or SMB when returning to menu
* TurboMode
* Global emulator preferences
* Menus redesigned (hopefully for the better!)
* Comes in 6 variants, each auto-loading/saving preferences/sram to a different
  location: mcslota, mcslotb, sdslota, sdslotb, smb, and noload
* ROM injector works again
* A number of small improvements and bug fixes
  
[2.0 WIP6]
* Port of SNES9X 1.50
* SMB support
* SD support
* Greatly improved emulation and timing for NTSC roms
* Save states (freezes) to SD and SMB
* Screen zoom
* Improved font and display
* ROM injector
* ... and more ...

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                                 CREDITS  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

                      Coding & menu design		Tantric
                      Additional coding			michniewski
                      Menu artwork				the3seashells
                      Menu sound				Peter de Man
                      
                      ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨
                      Snes9x GX GameCube		SoftDev, crunchy2, 
                      							eke-eke, others

                      Snes9x					Snes9x Team
                      libogc/devkitPPC			shagkur & wintermute
                      FreeTypeGX				Armin Tamzarian

•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•
|                                                                   LINKS  |
•˜———–—––-- - —————————––––– ———–—––-- - —————————––––– ———–—––-- - ————————•

											       SNES9X GX Project Page
							           http://code.google.com/p/snes9x-gx
							           
											       Wii Homebrew Community
											              www.wiibrew.org

											    Free (Public Domain) ROMs
											                www.pdroms.de
													       
											                  Cheat Codes
											           www.cheatzilla.com

×—–­—–­—–­—–­ –­—–­—–­—–­—–­—–­—–­—–­—–­—–­— ­—–­—–­—–­—–­—–­—–­—–­—-­—–­-–•¬
|0O×øo·                                                               ·oø×O0|
`¨•¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨ ¨¨¨¨¨¨¨¨¨¨¨¨¨'