//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "OsmAnnotatePlugin.h"

#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QRadialGradient>
#include <QtGui/QPushButton>
#include <QtGui/QPainterPath>
#include <QtGui/QFileDialog>

//#include <Phonon/MediaObject>
//#include <Phonon/VideoWidget>

#include <QtCore/QDebug>
#include <QtGui/QAction>
#include "ViewportParams.h"
#include "AbstractProjection.h"
#include "MarbleDirs.h"
#include "GeoPainter.h"
#include "GeoDataDocument.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPlacemark.h"
#include "TextAnnotation.h"
#include "GeoDataParser.h"
#include "AreaAnnotation.h"
#include "MarbleWidget.h"

namespace Marble
{

QStringList OsmAnnotatePlugin::backendTypes() const
{
    return QStringList( "osmannotation" );
}

QString OsmAnnotatePlugin::renderPolicy() const
{
    return QString( "ALWAYS" );
}

QStringList OsmAnnotatePlugin::renderPosition() const
{
    return QStringList( "ALWAYS_ON_TOP" );
}

QString OsmAnnotatePlugin::name() const
{
    return tr( "OSM Annotation Plugin" );
}

QString OsmAnnotatePlugin::guiString() const
{
    return tr( "&OSM Annotation Plugin" );
}

QString OsmAnnotatePlugin::nameId() const
{
    return QString( "osm-annotation-plugin" );
}

QString OsmAnnotatePlugin::description() const
{
    return tr( "This is a render and interaciton plugin used for annotating OSM data." );
}

QIcon OsmAnnotatePlugin::icon () const
{
    return QIcon();
}


void OsmAnnotatePlugin::initialize ()
{
    //initialise the first test widget
    QPushButton * button;

    button = new QPushButton(0);
    but = button;

    //Setup the model
    GeoDataCoordinates madrid( -13.7, 40.4, 0.0, GeoDataCoordinates::Degree );
    TextAnnotation* annon = new TextAnnotation();

    annon->setCoordinate(madrid);
    
    //FIXME memory leak withouth a model to do memory management
    model.append(annon);

    //Attempted to add a video widget as a tech preview but this needs to
    //be properly considered. Phonon includes in a plugin?
//    video = new Phonon::VideoWidget(0);
//
//    vid = video;
//
//    Phonon::VideoWidget * video;
//    Phonon::MediaObject m;
//    QString fileName("/home/foo/bar.ogg");
//
//    m.setCurrentSource(fileName);

    widgetInitalised= false;
    tmp_lineString = 0;
    m_document = 0;
    m_addPlacemark =0;
    m_drawPolygon = 0;
}

bool OsmAnnotatePlugin::isInitialized () const
{
    return true;
}

bool OsmAnnotatePlugin::render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer )
{
    if( !widgetInitalised ) {
        MarbleWidget* marbleWidget = (MarbleWidget*) painter->device();
        setupActions( marbleWidget );

        connect(this, SIGNAL(redraw()),
                marbleWidget, SLOT(repaint()) );

        widgetInitalised = true;
    }
    painter->autoMapQuality();

    //Set the parents if they have not already been set

//    if ( vid->parent() == 0 ) {
//        vid->setParent( (QWidget*)painter->device() );
//        //start video
//    }

    if ( but->parent() == 0 ) {

    }

    int x, y;
    bool hidden;

    GeoDataCoordinates madrid( -13.7, 40.4, 0.0, GeoDataCoordinates::Degree );
    
    QListIterator<TmpGraphicsItem*> i(model);
    
    while(i.hasNext()) {
        TmpGraphicsItem* tmp= i.next();
        tmp->paint(painter, viewport, renderPos, layer);
    }

    if ( tmp_lineString != 0 ) {
        painter->drawPolyline( *tmp_lineString );
    }




//    viewport->currentProjection()->screenCoordinates( madrid, viewport, x, y, hidden );

//    if( !hidden ) {
//        but->move(QPoint(x, y));
//        but->setVisible(false);
//    } else {
//        but->setVisible(false);
//    }

    //Figure out how to add the data parsed to a scene for rendering
    //FIXME: this is a terrible hack intended just to test!
    if( m_document ) {
        GeoDataPlacemark p(m_document->at(0));
        GeoDataLinearRing ring(* p.geometry() );

        painter->drawPolygon( ring );

    }

    return true;
}

void OsmAnnotatePlugin::drawPolygon(bool b)
{
    if( !b ) {
        //stopped drawing the polygon
        if ( tmp_lineString != 0 ) {
            AreaAnnotation* area = new AreaAnnotation();
            GeoDataPolygon poly( Tessellate );
            poly.setOuterBoundary( GeoDataLinearRing(*tmp_lineString) );
            delete tmp_lineString;
            tmp_lineString = 0;

            area->setGeometry( poly );

            model.append(area);

            //FIXME only redraw the new polygon
            emit(redraw());
        }
    }
}

void OsmAnnotatePlugin::loadOsmFile()
{
    QString filename;
    filename = QFileDialog::getOpenFileName(0, tr("Open File"),
                            QString(),
                            tr("All Supported Files (*.osm);;Open Street Map Data (*.osm)"));

    if ( ! filename.isNull() ) {

        GeoDataParser parser( GeoData_OSM );

        QFile file( filename );
        if ( !file.exists() ) {
            qWarning( "File does not exist!" );
            return;
        }

        // Open file in right mode
        file.open( QIODevice::ReadOnly );

        if ( !parser.read( &file ) ) {
            qWarning( "Could not parse file!" );
            //do not quit on a failed read!
            //return
        }
        GeoDocument* document = parser.releaseDocument();
        Q_ASSERT( document );

        m_document = static_cast<GeoDataDocument*>( document );

        file.close();

        qDebug() << "size of container is " << m_document->size();
    }
}

bool    OsmAnnotatePlugin::eventFilter(QObject* watched, QEvent* event)
{
    MarbleWidget* marbleWidget = (MarbleWidget*) watched;
    //FIXME why is the QEvent::MousePress not working? caught somewhere else?
    //does this mean we need to centralise the event handling?

    // Catch the mouse button press
    if ( event->type() == QEvent::MouseButtonPress ) {
        QMouseEvent* mouseEvent = (QMouseEvent*) event;

        // deal with adding a placemark
        if ( mouseEvent->button() == Qt::LeftButton
             && m_addPlacemark
             && m_addPlacemark->isChecked() )
        {
            //Add a placemark on the screen
            qreal lon, lat;

            bool valid = ((MarbleWidget*)watched)->geoCoordinates(((QMouseEvent*)event)->pos().x(),
                                                                  ((QMouseEvent*)event)->pos().y(),
                                                                  lon, lat, GeoDataCoordinates::Radian);
            if ( valid ) {
                GeoDataCoordinates point( lon, lat );
                TextAnnotation* t = new TextAnnotation();
                t->setCoordinate(point);
                model.append(t);

                //FIXME only repaint the new placemark
                ( ( MarbleWidget* ) watched)->repaint();
                m_addPlacemark->setChecked( false );
                return true;
            }


        }

        // deal with drawing a polygon
        if ( mouseEvent->button() == Qt::LeftButton
             && m_drawPolygon
             && m_drawPolygon->isChecked() )
        {
            qreal lon, lat;

            bool valid = ((MarbleWidget*)watched)->geoCoordinates( mouseEvent->pos().x(),
                                                                   mouseEvent->pos().y(),
                                                                   lon, lat, GeoDataCoordinates::Radian);
            if ( valid ) {
                if ( tmp_lineString == 0 ) {
                    tmp_lineString = new GeoDataLineString( Tessellate );
                }

                tmp_lineString->append(GeoDataCoordinates(lon, lat));

                //FIXME only repaint the line string so far
                marbleWidget->repaint();

            }
            return true;
        }

        //deal with clicking
        if ( mouseEvent->button() == Qt::LeftButton ) {
            qreal lon, lat;

        bool valid = ((MarbleWidget*)watched)->geoCoordinates(((QMouseEvent*)event)->pos().x(),
                                                              ((QMouseEvent*)event)->pos().y(),
                                                              lon, lat, GeoDataCoordinates::Radian);
        //if the event is in an item change cursor
        // FIXME make this more effecient by using the bsptree
        QListIterator<TmpGraphicsItem*> i(model);
        while(i.hasNext()) {
            TmpGraphicsItem* item = i.next();
            if(valid) {
                //FIXME check against all regions!
                QListIterator<QRegion> it ( item->regions() );

                while ( it.hasNext() ) {
                    QRegion p = it.next();
                    if( p.contains( mouseEvent->pos() ) ) {
                        return item->sceneEvent( event );
                    }
                }
            }

        }

        }
    }

//    // this stuff below is for hit tests. Just a sample mouse over for all bounding boxes
//    if ( event->type() == QEvent::MouseMove ||
//         event->type() == QEvent::MouseButtonPress ||
//         event->type() == QEvent::MouseButtonRelease ) {
//        QMouseEvent* mouseEvent = (QMouseEvent*) event;
//
//
//
//
//    }
    return false;
}

void OsmAnnotatePlugin::setupActions(MarbleWidget* widget)
{
    m_addPlacemark = new QAction(this);
    m_addPlacemark->setText( "Add Placemark" );
    m_addPlacemark->setCheckable( true );

    m_drawPolygon = new QAction( this );
    m_drawPolygon->setText( "Draw Polygon" );
    m_drawPolygon->setCheckable( true );
    connect( m_drawPolygon, SIGNAL(toggled(bool)),
             this, SLOT(drawPolygon(bool)) );

    m_loadOsmFile = new QAction( this );
    m_loadOsmFile->setText( "Load Osm File" );
    connect( m_loadOsmFile, SIGNAL(triggered()),
             this, SLOT(loadOsmFile()) );

    m_beginSeperator = new QAction( this );
    m_beginSeperator->setSeparator( true );
    m_endSeperator = new QAction ( this );
    m_endSeperator->setSeparator( true );

    widget->registerAction( m_beginSeperator );
    widget->registerAction( m_addPlacemark );
    widget->registerAction( m_drawPolygon );
    widget->registerAction( m_loadOsmFile );
    widget->registerAction( m_endSeperator );

}

}

Q_EXPORT_PLUGIN2( OsmAnnotatePlugin, Marble::OsmAnnotatePlugin )

#include "OsmAnnotatePlugin.moc"
