#include <stdio.h>
#include <stdlib.h>
#include "geo.h"


GeoIP *gi = NULL;
GeoIP *gi6 = NULL;


void geo_init() {
    gi = GeoIP_open("GeoLiteCity.dat", GEOIP_INDEX_CACHE);
    gi6 = GeoIP_open("GeoLiteCityv6.dat", GEOIP_INDEX_CACHE);
    if (gi == NULL || gi6 == NULL) {
        fprintf(stderr, "Error opening geoip databases\n");
        exit(1);
    }
}

void geo_close() {
    GeoIP_delete(gi);
    GeoIP_delete(gi6);
}

GeoIPRecord* geo_get(char* ip) {
    return GeoIP_record_by_name(gi, (const char *)ip);  // GeoIP_record_by_name_v6
    // must be freed later with GeoIPRecord_delete()
}

GeoIPRecord* geo_get6(char* ip) {
    return GeoIP_record_by_name_v6(gi6, (const char *)ip);
    // must be freed later with GeoIPRecord_delete()
}

const char* geo_country_name(GeoIPRecord* rec) {
    return GeoIP_country_name_by_id(gi, GeoIP_id_by_code(rec->country_code));
}

#ifdef TEST
static const char * _mk_NA( const char * p ){
    return p ? p : "N/A";
}

int main(int argc, char** argv) {
    geo_init();
    char* host = "24.4.129.164";
    // char* host6 = "2601:647:4701:733:5bf:f3c2:f2b2:9c1f";

    GeoIPRecord *gir = GeoIP_record_by_name(gi, (const char *) host);  // GeoIP_record_by_name_v6
    // GeoIPRecord *gir = GeoIP_record_by_name_v6(gi, (const char *) host6);

    printf("%s\t%s\t%s\t%s\t%s\t%s\t%f\t%f\t%d\t%d\n", host,
    _mk_NA(gir->country_code),
    _mk_NA(gir->region),
    _mk_NA(GeoIP_region_name_by_code(gir->country_code, gir->region)),
    _mk_NA(gir->city),
    _mk_NA(gir->postal_code),
    gir->latitude,
    gir->longitude,
    gir->metro_code,
    gir->area_code);

    GeoIPRecord_delete(gir);

    geo_close();
}
#endif
