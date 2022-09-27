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
ESI MAYA44
ESI Prodigy 7.1 Hi-Fi
ESI Prodigy Hi-Fi
Audiotrack Prodigy 7.1
Audiotrack Prodigy HD2

Cards that are not listed may not work at all or work with some limitations, be carefoul!

"

if [ -d /Library/Extensions/Envy24HTPCIAudioDriver.kext ]; then
	kextunload /Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	kextunload /Library/Extensions/Envy24HTPCIAudioDriver.kext > /dev/null
	
	rm -R /Library/Extensions/Envy24HTPCIAudioDriver.kext
fi

if [ -d /Library/Extensions/Envy24HTAudioDriver_64bit.kext ]; then
    kextunload /Library/Extensions/Envy24HTAudioDriver_64bit.kext > /dev/null
    kextunload /Library/Extensions/Envy24HTAudioDriver_64bit.kext > /dev/null
    kextunload /Library/Extensions/Envy24HTAudioDriver_64bit.kext > /dev/null
    kextunload /Library/Extensions/Envy24HTAudioDriver_64bit.kext > /dev/null

    rm -R /Library/Extensions/Envy24HTAudioDriver_64bit.kext
fi

cp -R Envy24HTAudioDriver_64bit.kext /Library/Extensions/

find /Library/Extensions/Envy24HTAudioDriver_64bit.kext -type d -exec /bin/chmod 0755 {} \;
find /Library/Extensions/Envy24HTAudioDriver_64bit.kext -type f -exec /bin/chmod 0744 {} \;
chown -R root:wheel /Library/Extensions/Envy24HTAudioDriver_64bit.kext

kextload -t /Library/Extensions/Envy24HTAudioDriver_64bit.kext

echo "Installation finished"
