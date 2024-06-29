-------------------------------------------------------
Led Blur (Atari/Firebee port) - by MiKRO / Mystic Bytes
-------------------------------------------------------

Sometimes during Fall of 2020 I bumped into this demo on pouet. Seeing how easy
it would be to port it over I did exactly that -- only to find out that there
are some bugs related to big endianness of our CPU (all the other ports were
done for little endian machines...). I gave it a few more days and gave up.

Oddly enough, today I was browsing my folder dedicated to all sorts of
unfinished things and found this one. Of course I fixed all bugs in like 20
minutes and demo was good to go. :)

The demo uses a CPU-only mod player so that's not exactly helping
performance-wise. On the other hand, this demo is fully system and GEM
compliant: it runs in a window, too! (if you want to run it fullscreen under
FreeMiNT, make sure that "SDL_VIDEODRIVER" environment variable is not set to
"gem").

Use '-h' command line parameter for list of available options.

Oh and there's a Firebee provided as well.

Source code can be found here: https://github.com/mikrosk/led_blur.

Miro Kropacek aka MiKRO / Mystic Bytes
10.09.2023, Kosice / Slovakia
http://mikro.atari.org

Update 22.12.2023:
------------------

After some feedback I have assembled a new version with following updates:

- fixed sound on Milan / Hades (both MilanBlaster and GSXB are supported)
- fixed SDL_VIDEODRIVER=xbios on CTPCI, Milan and TT+NOVA graphics card
  (unfortunately on Milan it seems to require some special XBIOS driver or 
  upgrade; maybe someone with S3 card can let me know how it works there)
- fixed Falcon RGB monitor output
- fixed SuperVidel monitor output
- optimised the RGB565 draw loop
- optimised audio sample buffer (rendering isn't affected by playback anymore)
- disabled FPS display and added '-p' parameter to bring it back
- added '-d' to see the effect of double buffering (xbios only)
- added a version for graphics card with little endian pixel format
- renamed the mod file to an 8+3 filename

Please note that the audio driver is custom, i.e. SDL_AUDIODRIVER has no
effect. I tried it but it was like 10x slower, perhaps some internal resampling
was taking place (even if it shouldn't have). I have reported it to Patrice
Mandin.

SDL_VIDEODRIVER=gem performs best in fullscreen ('-f') however there is
currently a bug in SDL which prevents the demo from exiting in such setup.

The best results you get on a computer which is able to provide 16-bit (RGB565)
pixel format; it will run on Falcon VGA's 640x480@8-bit or TT's 320x480@8-bit
for example but the performance will be just terrible (realtime dithering...).

When looking for system-friendly audio XBIOS handling, take a look here:
https://github.com/mikrosk/atari_sound_setup. Hopefully one day it will find
its way into SDL.

The demo uses a modified version of libxmp-lite, patched with support for
byte swapping: https://github.com/mikrosk/build-scripts/blob/master/libxmp-lite.patch.

lb_be.tos: Atari version, standard (big endian) pixel format support.
lb_le.tos: Atari version, NOVA/ET4000 (little endian) pixel format support.
lb_fb.tos: FireBee version.

Update XX.XX.2024:
------------------

- one version for little and big endian pixel format

lb.tos: Atari (68020+FPU) version.
lb_fb.tos: FireBee version.

------------------------------------------------
Led Blur (Windows port) - by Optimus / Mindlapse
------------------------------------------------

This is the PC port of my GP32 demo "Led Blur". I originally started this one
as a wrapper of some GP32SDK functions over SDL to help me developing easier on
PC. Later, I thought it would be a good idea to use this code in order to
release the PC port of this demo. Not many sceners have a GP32 and the one and
only GP32 emulator is not suitable for demowatching. So, even PC sceners can
enjoy my demo.


---------------------------------------------
Led Blur (UNIX port) - by Nuclear / Mindlapse
---------------------------------------------
Precompiled binaries are included for GNU/Linux x86/x86_64, and SGI IRIX mips3.
Just run the ./ledblur_unix and it will execute the appropriate binary.

The UNIX port should compile and run on any UNIX operating system and architecture.
To compile just type make or gmake (not sure if it works with non-GNU make), and run
the ./opti_ledblur binary.

For anything regarding the UNIX port, contact: nuclear@siggraph.org


=======================

It's finally here..

After one year of laziness, I've managed to finish my first (and last it seems ;)
GP32 demo. It's released under the Mindlapse group. (http://mindlapse.demoscene.gr)

Still full of several glitches I couldn't correct, though it had to be released
one day. The source code is availiable for anyone who wants to play with it,
correct few bugs or decides to make it BLU+ compatible (sorry).

Special thanks to The Hardliner for his awesome music support! =)

Optimus/Mindlapse
optimus6128@yahoo.gr

p.s. Moving on to GP2X!
