//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008-2009      Patrick Spendrin <ps_ml@gmx.de>
//


// Own
#include "GeoDataPlacemark.h"

// Qt
#include <QtCore/QDataStream>
#include <QtCore/QDebug>

// Private
#include "GeoDataPlacemark_p.h"

namespace Marble
{

GeoDataPlacemark::GeoDataPlacemark( GeoDataObject* parent )
    : GeoDataFeature( new GeoDataPlacemarkPrivate )
{
}

GeoDataPlacemark::GeoDataPlacemark( const GeoDataPlacemark& other )
: GeoDataFeature( other )
{
}

GeoDataPlacemark::GeoDataPlacemark( const GeoDataFeature& other )
: GeoDataFeature( other )
{
}

GeoDataPlacemark::GeoDataPlacemark( const QString& name, GeoDataObject *parent )
    : GeoDataFeature( name )
{
}

GeoDataPlacemark::~GeoDataPlacemark()
{
}

GeoDataPlacemarkPrivate* GeoDataPlacemark::p() const
{
    return static_cast<GeoDataPlacemarkPrivate*>(d);
}

GeoDataGeometry* GeoDataPlacemark::geometry() const
{
    if( p()->m_geometry )
        return p()->m_geometry;
    else
        return &( p()->m_coordinate );
}

GeoDataCoordinates GeoDataPlacemark::coordinate() const
{
    return static_cast<GeoDataCoordinates>( p()->m_coordinate );
}

void GeoDataPlacemark::coordinate( qreal& lon, qreal& lat, qreal& alt ) const
{
    p()->m_coordinate.geoCoordinates( lon, lat );
    alt = p()->m_coordinate.altitude();
}

void GeoDataPlacemark::setCoordinate( qreal lon, qreal lat, qreal alt )
{
    detach();
    p()->m_coordinate = GeoDataPoint( lon, lat, alt, this );
}

void GeoDataPlacemark::setCoordinate( const GeoDataPoint &point )
{
    detach();
    p()->m_coordinate = GeoDataPoint( point );
    p()->m_coordinate.setParent( this );
}

void GeoDataPlacemark::setGeometry( const GeoDataPoint& point )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = new GeoDataPoint( point );
}

void GeoDataPlacemark::setGeometry( const GeoDataLineString& point )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = new GeoDataLineString( point );
}

void GeoDataPlacemark::setGeometry( const GeoDataLinearRing& point )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = new GeoDataLinearRing( point );
}

void GeoDataPlacemark::setGeometry( const GeoDataPolygon& point )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = new GeoDataPolygon( point );
}

void GeoDataPlacemark::setGeometry( const GeoDataMultiGeometry& point )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = new GeoDataMultiGeometry( point );
}

qreal GeoDataPlacemark::area() const
{
    return p()->m_area;
}

void GeoDataPlacemark::setArea( qreal area )
{
    detach();
    p()->m_area = area;
}

qint64 GeoDataPlacemark::population() const
{
    return p()->m_population;
}

void GeoDataPlacemark::setPopulation( qint64 population )
{
    detach();
    p()->m_population = population;
}

const QString GeoDataPlacemark::countryCode() const
{
    return p()->m_countrycode;
}

void GeoDataPlacemark::setCountryCode( const QString &countrycode )
{
    detach();
    p()->m_countrycode = countrycode;
}

void GeoDataPlacemark::pack( QDataStream& stream ) const
{
    GeoDataFeature::pack( stream );

    stream << p()->m_countrycode;
    stream << p()->m_area;
    stream << p()->m_population;

    stream << p()->m_geometry->geometryId();
    p()->m_geometry->pack( stream );
    p()->m_coordinate.pack( stream );
}


void GeoDataPlacemark::unpack( QDataStream& stream )
{
    detach();
    GeoDataFeature::unpack( stream );

    stream >> p()->m_countrycode;
    stream >> p()->m_area;
    stream >> p()->m_population;

    int geometryId;
    stream >> geometryId;
    switch( geometryId ) {
        case InvalidGeometryId:
            break;
        case GeoDataPointId:
            {
            GeoDataPoint* point = new GeoDataPoint;
            point->unpack( stream );
            delete p()->m_geometry;
            p()->m_geometry = point;
            }
            break;
        case GeoDataLineStringId:
            {
            GeoDataLineString* lineString = new GeoDataLineString;
            lineString->unpack( stream );
            delete p()->m_geometry;
            p()->m_geometry = lineString;
            }
            break;
        case GeoDataLinearRingId:
            {
            GeoDataLinearRing* linearRing = new GeoDataLinearRing;
            linearRing->unpack( stream );
            delete p()->m_geometry;
            p()->m_geometry = linearRing;
            }
            break;
        case GeoDataPolygonId:
            {
            GeoDataPolygon* polygon = new GeoDataPolygon;
            polygon->unpack( stream );
            delete p()->m_geometry;
            p()->m_geometry = polygon;
            }
            break;
        case GeoDataMultiGeometryId:
            {
            GeoDataMultiGeometry* multiGeometry = new GeoDataMultiGeometry;
            multiGeometry->unpack( stream );
            delete p()->m_geometry;
            p()->m_geometry = multiGeometry;
            }
            break;
        case GeoDataModelId:
            break;
        default: break;
    };
    p()->m_coordinate.unpack( stream );
}

}
