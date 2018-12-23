# speedTestServer
A Simple HTTP Server written by C for network speed test, less than 900K memory needed.

# Compile & Install

```
git clone git@github.com:linexjlin/speedTestServer.git
make
sudo make install
```

# Usage 
## Run Server

```
spd -p 80  #The default port is 10000.
```

## Brower Test Files
On brower open your.server.addr.

## Download Test
wget -O /dev/null http://your.server.addr/128M 
## Uninstall

```
make uninstal 
```
