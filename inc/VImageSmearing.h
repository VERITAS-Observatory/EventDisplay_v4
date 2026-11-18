#ifndef VIMAGESMEARING_H
#define VIMAGESMEARING_H

#include <algorithm>
#include <cmath>
#include <cstdint>

// Version 1: event-keyed SplitMix64 and Box-Muller draws. These are not NumPy draws.
namespace VImageSmearing
{
struct Settings
{
    uint32_t seed;
    double fraction, size, width, length, centroid, asym, tgrad;
};
struct Image
{
    double size, width, length, cen_x, cen_y, dist, asym, tgrad;
};
inline uint64_t mix( uint64_t x )
{
    x += UINT64_C(0x9e3779b97f4a7c15);
    x = ( x ^ ( x >> 30 ) ) * UINT64_C(0xbf58476d1ce4e5b9);
    x = ( x ^ ( x >> 27 ) ) * UINT64_C(0x94d049bb133111eb);
    return x ^ ( x >> 31 );
}
inline double uniform( uint64_t key )
{
    return ( ( mix( key ) >> 11 ) + 0.5 ) * ( 1.0 / 9007199254740992.0 );
}
inline double gaussian( uint64_t key )
{
    return std::sqrt( -2. * std::log( uniform( key ) ) ) *
           std::cos( 6.2831853071795864769 * uniform( key ^ UINT64_C(0xd1b54a32d192ed03) ) );
}
inline void apply( Image& image, const Settings& c, uint64_t run, uint64_t event, unsigned int tel )
{
    if( !std::isfinite( image.size ) || image.size <= 0. ) return;
    const uint64_t eventKey = mix( c.seed ) ^ mix( run ) ^ mix( event + UINT64_C(0x632be59bd9b4e019) );
    if( uniform( eventKey ) >= c.fraction ) return;
    const uint64_t key = mix( eventKey ^ mix( tel ) );
    if( c.size ) image.size *= std::pow( 10., c.size * gaussian( key ^ 1 ) );
    if( c.width && std::isfinite( image.width ) && image.width > 0. )
        image.width = std::max( 1.e-6, image.width + c.width * gaussian( key ^ 2 ) );
    if( c.length && std::isfinite( image.length ) && image.length > 0. )
        image.length = std::max( 1.e-6, image.length + c.length * gaussian( key ^ 3 ) );
    if( c.centroid && std::isfinite( image.cen_x ) && std::isfinite( image.cen_y ) )
    {
        image.cen_x += c.centroid * gaussian( key ^ 4 );
        image.cen_y += c.centroid * gaussian( key ^ 5 );
        image.dist = std::hypot( image.cen_x, image.cen_y );
    }
    if( c.asym && std::isfinite( image.asym ) ) image.asym += c.asym * gaussian( key ^ 6 );
    if( c.tgrad && std::isfinite( image.tgrad ) ) image.tgrad += c.tgrad * gaussian( key ^ 7 );
}
}
#endif
