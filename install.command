#!/bin/sh

cd "$( dirname "$0" )"

if [ -x /usr/bin/sudo -a $UID -ne 0 ]; then
   exec /usr/bin/sudo $0 $*
fi

sysctl -w debug.iokit=0x200000

echo "This will install the Envy24HT driver on your system.

Currently supported cards are:
Terratec Aureon 5.1 Sky
Terratec Aureon 7.1 Space
Terratec Phase22
Terratec Phase28
M-Audio Revolution 5.1
M-Audio Revolution 7.1
M-Audio Audiophile 192
ESI Juli@
ESI Prodigy Hi-Fi
Audiotrack Prodigy 7.1
Audiotrack Prodigy HD2

Cards that are not listed may not work at all or work with some limitations, be carefoul!

"

if [ -d /System/Library/Extensions/Envy24HTPCIAudioDriver.kext ]; then
	kextunload /System/Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /System/Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /System/Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /System/Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	
	rm -R /System/Library/Extensions/Envy24HTPCIAudioDriver.kext
fi

cp -R Envy24HTPCIAudioDriver.kext /System/Library/Extensions/

find /System/Library/Extensions/Envy24HTPCIAudioDriver.kext -type d -exec /bin/chmod 0755 {} \;
find /System/Library/Extensions/Envy24HTPCIAudioDriver.kext -type f -exec /bin/chmod 0744 {} \;
chown -R root:wheel /System/Library/Extensions/Envy24HTPCIAudioDriver.kext

kextload -t /System/Library/Extensions/Envy24HTPCIAudioDriver.kext

echo "Installation finished"
