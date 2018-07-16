FROM scratch

ADD csyslog /csyslog
ADD GeoLiteCity.dat /
ADD GeoLiteCityv6.dat /

STOPSIGNAL 9

ENTRYPOINT ["/csyslog"]
