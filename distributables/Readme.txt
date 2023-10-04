requires an online connection

defaults to mapbox satellite maps requiring an access token
(free as of writing this line)

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
