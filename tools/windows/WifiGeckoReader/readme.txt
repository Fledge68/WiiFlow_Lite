WifiGeckoReader allows you to view the gecko debug messages over the wifi connection. Only use this when debugging.  There is no need to use it regularly.

It uses UDP port 4405.

To make it work with wiiflow, edit or add the following wiiflow.ini lines:
[GENERAL]
async_network=yes
wifi_gecko=yes
wifi_gecko_port=4405

wifi_gecko_ip=your.computer.ipAddr.here
eg:
wifi_gecko_ip=192.168.0.199

Once this is done, run WifiGeckoReader, then run wiiflow.  You should start seeing debug messages.
