# Raspberry Pi configuration for Senpaï robots
This is a recap of all the configuration performed on the raspberry pi.  
The folder `config-files` contains templates for config files to use.  
The folder `senpai-daemon` contains all the utility scripts. It should be copied to `/home/pi/` on the Raspberry pi.


## Network configuration

### Wifi auto-connect to network
Edit the file:
`/etc/wpa_supplicant/wpa_supplicant.conf`

### DHCP client
Config file:
`/etc/dhcpcd.conf`

Serive name:
`dhcpcd`

### DHCP server
Config file:
`/etc/dnsmasq.conf`

Service name:
`dnsmasq`

### Hotspot
Config file:
`/etc/hostapd/hostapd.conf`

Service name:
`hostapd`


## Run `senpai-daemon.sh` on startup
Edit the file `/etc/rc.local` and add the following line:  
`/home/pi/senpai-daemon/senpai-daemon.sh &`

## LowLevelServer
Clone `https://github.com/sylvaing19/LowLevelServer.git`  
Install `make` and `cmake`  
Compile: `./cli-build.sh`  
Copy the executable `LowLevelServer` to the folder `senpai-daemon/bin/`  
Config file (to set IP address, TCP port, etc...):
`senpai-daemon/conf/lowlevelserver.conf`

## Pin mapping

| Function     | Wiring Pi pin | BCM pin 6 | Physical pin |
|--------------|---------------|-----------|--------------|
| Halt btn     | 21            | 5         | 29           |
| Halt LED     | 22            | 6         | 31           |
| Network1 btn | 0             | 17        | 11           |
| Network2 btn | 3             | 22        | 15           |
| Network1 LED | 2             | 27        | 13           |
| Network2 LED | 4             | 23        | 16           |
| Function btn | 25            | 26        | 37           |
| Function LED | 23            | 13        | 33           |
