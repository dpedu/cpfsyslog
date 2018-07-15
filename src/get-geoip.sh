#!/bin/sh

set -x
set -e

wget -N http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
wget -N http://geolite.maxmind.com/download/geoip/database/GeoLiteCityv6-beta/GeoLiteCityv6.dat.gz

rm -vf *.dat
gunzip --keep Geo*.gz
