requires an online connection*


1.

 a)  defaults to mapbox satellite map tiles as texture requiring an access token
     (free as of writing this line: https://account.mapbox.com/auth/signup)

 b)  put access token / api key (=: key) into file mapservice-key.txt
     on first line, save as UTF-8 encoded Unicode text

 c)  keys of length up to and including 4096 bytes (around 4096 latin characters)
     are supported, key may not contain a % character

 d)  a cache of textures downloaded is created in folder "cache" in the directory
     Globe is executing in


2.

 a)  a raster tiles map of any service is supported as texture when
     - service delivers 256x256 sized JPEGs
     - service url identifies map tiles by a z/x/y scheme
     - service url does not contain a % character
     - service url is below or equal 4096 bytes in size
     - service map tile identified by 0/0/0 spans longitudes
       from -180° at the left to +180° at the right and latitudes
       from around +85° at the top to around -85° at the bottom
     - service's map is web-mercator-projected

 b)  put service url in file mapservice-url.txt on first line,
     do not include protocol and ://, so no https://,
     replace z/x/y map tile identifying url part by ^^,
     if required, replace key url part by <<,
     save as UTF-8 encoded Unicode text


3.

control the globe with W, A, S, D and arrow keys on keyboard,
control the globe with your mouse


4.

 a)  if zooming closer results in a black image, the map service does not support
     that zoom level, Globe supports zoom level from including 0 to including 30

 b)  Globe not informs about failed retrieval of map tiles, e.g. when the key is
     invalid; then nothing gets rendered and the window remains black


5.

 a)  around the poles the data of the closest latitude having data available is
     stretched to them

 b)  elevation is presented for outer zoom levels and is exaggerated by a factor
     of 40


6.

if the application does not start, you might need the Visual Studio 2022
X64 Visual C(++) Redistributable you can find at
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
as of writing this line


7.

elevation data averaged from https://ngdc.noaa.gov/mgg/topo/globe.html



*you can download raster tiles map tiles and setup a server for them locally
 and put the url to them into the mapservice-url.txt file like described in
 this Readme.txt file; describing setting up a server is out of scope of this
 project