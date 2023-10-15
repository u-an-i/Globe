requires an online connection*


defaults to mapbox satellite map tiles as texture requiring an access token
(free as of writing this line: https://account.mapbox.com/auth/signup)

put access token / api key (=: key) into file mapservice-key.txt
on first line, save as UTF-8 encoded Unicode text

keys of length up to and including 4096 bytes (around 4096 latin characters)
are supported, key may not contain a % character


a raster tiles map of any service is supported as texture when
- service delivers 256x256 sized JPEGs
- service url identifies map tiles by a z/x/y scheme
- service url does not contain a % character
- service url is below or equal 4096 bytes in size
- service map tile identified by 0/0/0 spans longitudes
  from -180° at the left to +180° at the right and latitudes
  from around +85° at the top to around -85° at the bottom
- service's map is web-mercator-projected

put service url in file mapservice-url.txt on first line,
do not include protocol and ://, so no https://,
replace z/x/y map tile identifying url part by ^^,
if required, replace key url part by <<,
save as UTF-8 encoded Unicode text


control globe with W, A, S, D and arrow keys on keyboard,
control globe with your mouse


if zooming closer results in a black image, the map service does not support
that zoom level, this application supports zoom level from including 0 to
including 30

the application not informs about failed retrieval of map tiles, e.g. when
the key is invalid; then nothing gets rendered and the window remains black


around the poles the data of the closest latitude having data available is
stretched to them

elevation is presented for outer zoom levels and is exaggerated by a factor
of 20


if the application does not start, you might need the Visual Studio 2022
X64 Visual C(++) Redistributable you can find at
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
as of writing this line


elevation data averaged from https://ngdc.noaa.gov/mgg/topo/globe.html



*you can download a raster tiles map and setup a server for them locally
 and put the url to them into the mapservice-url.txt file like described
 in this Readme.txt file; describing setting up a server for them is out
 of scope of this project