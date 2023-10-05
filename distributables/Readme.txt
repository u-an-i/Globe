requires an online connection

defaults to mapbox satellite maps requiring an access token
(free as of writing this line: https://account.mapbox.com/auth/signup)

put access token / api key (=: key) into file mapservice-key.txt
on first line, save as UTF-8 encoded Unicode text

keys of length up to and including 4096 bytes
(around 4096 latin characters) are supported,
key may not contain a % character

raster tile maps of any service are supported when
service delivers 256x256 sized JPEGs
service url identifies map tiles by a z/x/y scheme
service url does not contain a % character
service url is below or equal 4096 bytes in size

put service url in file mapservice-url.txt on first line,
do not include protocol and ://, so no https://,
replace z/x/y map tile identifying url part by ^^,
if required, replace key url part by <<,
save as UTF-8 encoded Unicode text

control globe by W, A, S, D and arrow keys on keyboard

if zooming closer results in a black image the map service
does not support that zoom level

the application not informs about failed retrieval of map tiles
eg when the key is invalid, the window remains black in such case

if the application does not start, you might need the Visual Studio 2022
X64 Visual C(++) Redistributable you can find at
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
as of writing this line