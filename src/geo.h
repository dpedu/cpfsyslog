#include <GeoIP.h>
#include <GeoIPCity.h>

void geo_init();
void geo_close();
GeoIPRecord* geo_get(char* ip);
GeoIPRecord* geo_get6(char* ip);
const char* geo_country_name(GeoIPRecord* rec);
