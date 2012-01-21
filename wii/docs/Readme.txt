NOTE: Also see http://www.wiiflowiki.com

    WiiFlow  Manual


  Content
 =========

1.1) About the Installation from WiiFlow
1.2) Installation of WiiFlow
1.3) Starting WiiFlow

2.1) Installing themes
2.2) Playing background music

  1.1 About the Installation of WiiFlow
 -------------------------------
You can either install WiiFlow to a SD-Card or on your Harddisk. Both types has 
advantages and disadvantages: If you install on a SD-Card you will need the 
SD-Card to operate with WiiFlow. With this setup, you must always let the SD-Card inside
your Wii and with write-protect off. Otherwise WiiFlow will not work properly.
Also, with covers and settings and fanart/trailers/etc an SD will become full very fast.

It is better to install WiiFlow on your Harddisk but you will need a FAT32, NTFS, or EXT2/3/4
partition for it. Since HBC does not support ntfs, a forwarder is required to boot a dol which
is placed on an NTFS partition.

If you have a WBFS only hard disk, you would need to repartition it, which will delete all
of your Wii-Games on it. If you do this we suggest you backup as many of your games as you can.

  1.2 Installation of WiiFlow:
 ------------------------------------------
NOTE: DO NOT boot wiiflow until the entire installation is completed.
If you have a previous installation, it is recommended to back up your covers and delete 
all wiiflow folders before you continue.

Copy the content of the archive to the Root of the device you wish to use to boot wiiflow.
You should get the following Path:

device:\apps\wiiflow\wiiflow.ini

WiiFlow will save all settings and files into the data directory by default, which is: device:\wiiflow\.

If you want the data on a different device than where you have the apps folder,
open the included device:/apps/wiiflow/wiiflow.ini and change the "data_on_usb=" option.

If you set "data_on_usb=yes", move the device:/wiiflow folder along with its contents to USB.
If you set "data_on_usb=no", move the device:/wiiflow folder along with its contents to SD.

You may now boot wiiflow.

The initial boot time of wiiflow may seem slow as it creates files and folders for the installation, this is normal.
Once you get into the coverflow, you may either exit and add in your covers back to the device (If you have them), or 
click the settings icon (The icon with the little gear on it in the bottom left corner of the screen) and
select from the menu that pops up "Download covers and titles".  This will bring up yet another menu where you must select "WiiTDB".
Wait for it to complete, press B to go back or click A on the back button, and then select the option "Missing" and wait for that to also complete.
Wiiflow is now set up for basic use.

  1.3 Starting WiiFlow
 ----------------------
1.3.1) Starting WiiFlow via Homebrew-Channel
You need to save the file "boot.dol" into a FAT partition in the directory device:\apps\wiiflow\ 
so that the Homebrew-Channel can find it. The files meta.xml and icon.png belongs in 
that directory too, but they are not necessary for the execution.

1.3.2) Starting WiiFlow through a Forwarder-Channel
A forwarder is a channel which executes a specific file in a specific directory. 
So you may need to rename the boot.dol and place it on that directory where the 
forwarder can find it. Ask the author of the forwarder which file it will start.

1.3.3) Starting WiiFlow with help of Preloader
You may use a forwarder designed for priiloader, or you may install the boot.dol directly as 
an autoboot file.  
To use the boot.dol itself:
Rename boot.dol to WiiFlow.dol and save it inside the root of your SD-Card. Now 
start Preloader and select "Load/Install File" and then the "WiiFlow.dol". 
After a short time the file is installed and you can automatically start 
WiiFlow by powering on your Wii if you set this in the settings of Preloader.


  2.1 Installing Themes
 ---------------------------
You can change the look of WiiFlow by installing an additional theme. You can 
either download a theme or create one by yourself. To install a theme just copy 
it inside the directory datadevice:\wiiflow\themes\. Themes consist of a ini-file and a 
belonging directory, which contains the graphics and fonts. You only have to 
check if both has the same name (e.g. to the file "\wiiflow\themes\pear.ini" 
belongs the directory "\wiiflow\themes\pear\" and all files inside of it). 
After you have copied both things, you can choose this theme in the settings. 

  2.2 Playing background-music
 ------------------------------
If you want to have some music in WiiFlow, you can save one or more audio 
files inside of \wiiflow\music\. WiiFlow will randomly play one of these 
files each time you start it. If the file is played to the end of it, it 
will start a new one.  WiiFlow will only play 
OGG-files and MP3's.