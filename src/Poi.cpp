/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * Note:     This work is based on the original RoutePoint.cpp file which is:
 * Copyright (C) 2013 by David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include "wx/wxprec.h"

#include <wx/dcscreen.h>
#include <wx/tokenzr.h>
#include <wx/listimpl.cpp>

#include "PoiMan.h"
#include "Poi.h"

//#include "navutil.h"


extern PoiMan *pPoiMan;

// todo do we really need these?
bool g_bIsNewLayer;
int g_LayerIdx;


WX_DEFINE_LIST ( PoiList );
//WX_DEFINE_LIST ( HyperlinkList );

Poi::Poi()
{
    m_pbmIcon = NULL;

    //  Nice defaults
    m_seg_len = 0.0;
    m_seg_vmg = 0.0;
    m_seg_etd = wxInvalidDateTime;
    m_bDynamicName = false;
    m_bPtIsSelected = false;
    m_bIsBeingEdited = false;
    m_bIsActive = false;
    m_bBlink = false;
    m_bIsInRoute = false;
    m_bIsInTrack = false;
    m_CreateTimeX = wxDateTime::Now();
    m_GPXTrkSegNo = 1;
    m_bIsolatedMark = false;
    m_bShowName = true;
    m_bKeepXRoute = false;
    m_bIsVisible = true;
    m_bIsListed = true;
    CurrentRect_in_DC = wxRect( 0, 0, 0, 0 );
    m_NameLocationOffsetX = -10;
    m_NameLocationOffsetY = 8;
    m_pMarkFont = NULL;
    m_btemp = false;
    m_SelectNode = NULL;
    m_ManagerNode = NULL;
    
    m_HyperlinkList = new HyperlinkList;

    //m_GUID = pPoiMan->CreateGUID( this );

    m_IconName = wxEmptyString;
//    ReLoadIcon();

    m_MarkName = wxEmptyString;

    m_bIsInLayer = false;
    m_LayerID = 0;
}

// Copy Constructor
Poi::Poi( Poi* orig )
{
    m_MarkName = orig->GetName();
    m_lat = orig->m_lat;
    m_lon = orig->m_lon;
    m_seg_len = orig->m_seg_len;
    m_seg_vmg = orig->m_seg_vmg;
    m_seg_etd = orig->m_seg_etd;
    m_bDynamicName = orig->m_bDynamicName;
    m_bPtIsSelected = orig->m_bPtIsSelected;
    m_bIsBeingEdited = orig->m_bIsBeingEdited;
    m_bIsActive = orig->m_bIsActive;
    m_bBlink = orig->m_bBlink;
    m_bIsInRoute = orig->m_bIsInRoute;
    m_bIsInTrack = orig->m_bIsInTrack;
    m_CreateTimeX = orig->m_CreateTimeX;
    m_GPXTrkSegNo = orig->m_GPXTrkSegNo;
    m_bIsolatedMark = orig->m_bIsolatedMark;
    m_bShowName = orig->m_bShowName;
    m_bKeepXRoute = orig->m_bKeepXRoute;
    m_bIsVisible = orig->m_bIsVisible;
    m_bIsListed = orig->m_bIsListed;
    CurrentRect_in_DC = orig->CurrentRect_in_DC;
    m_NameLocationOffsetX = orig->m_NameLocationOffsetX;
    m_NameLocationOffsetY = orig->m_NameLocationOffsetY;
    m_pMarkFont = orig->m_pMarkFont;
    m_MarkDescription = orig->m_MarkDescription;
    m_btemp = orig->m_btemp;

    m_HyperlinkList = new HyperlinkList;
    m_IconName = orig->m_IconName;
//    ReLoadIcon();

    m_bIsInLayer = orig->m_bIsInLayer;
    //m_GUID = pPoiMan->CreateGUID( this );
    
    m_SelectNode = NULL;
    m_ManagerNode = NULL;
    
}

Poi::Poi( double lat, double lon, const wxString& icon_ident, const wxString& name,
        const wxString &pGUID, bool bAddToList )
{
    //  Establish points
    m_lat = lat;
    m_lon = lon;

    //      Normalize the longitude, to fix any old poorly formed points
    if( m_lon < -180. ) m_lon += 360.;
    else
        if( m_lon > 180. ) m_lon -= 360.;

    //  Nice defaults
    m_seg_len = 0.0;
    m_seg_vmg = 0.0;
    m_seg_etd = wxInvalidDateTime;
    m_bDynamicName = false;
    m_bPtIsSelected = false;
    m_bIsBeingEdited = false;
    m_bIsActive = false;
    m_bBlink = false;
    m_bIsInRoute = false;
    m_bIsInTrack = false;
    m_CreateTimeX = wxDateTime::Now();
    m_GPXTrkSegNo = 1;
    m_bIsolatedMark = false;
    m_bShowName = true;
    m_bKeepXRoute = false;
    m_bIsVisible = true;
    m_bIsListed = true;
    CurrentRect_in_DC = wxRect( 0, 0, 0, 0 );
    m_NameLocationOffsetX = -10;
    m_NameLocationOffsetY = 8;
    m_pMarkFont = NULL;
    m_btemp = false;

    m_SelectNode = NULL;
    m_ManagerNode = NULL;
    
    m_HyperlinkList = new HyperlinkList;

    if( !pGUID.IsEmpty() )
        m_GUID = pGUID;
    //else
        //m_GUID = pPoiMan->CreateGUID( this );

    //      Get Icon bitmap
    m_IconName = icon_ident;
    //ReLoadIcon();

    SetName( name );
    //  Possibly add the waypoint to the global list maintained by the waypoint manager

    if( bAddToList && NULL != pPoiMan )
        pPoiMan->AddRoutePoint( this );

    m_bIsInLayer = g_bIsNewLayer;
    if( m_bIsInLayer ) {
        m_LayerID = g_LayerIdx;
        m_bIsListed = false;
    } else
        m_LayerID = 0;
}

Poi::~Poi( void )
{
//  Remove this point from the global waypoint list
    if( NULL != pPoiMan )
        pPoiMan->RemoveRoutePoint( this );

    if( m_HyperlinkList ) {
        m_HyperlinkList->DeleteContents( true );
        delete m_HyperlinkList;
    }
}

wxDateTime Poi::GetCreateTime()
{
    if(!m_CreateTimeX.IsValid()) {
        if(m_timestring.Len())
            ParseGPXDateTime( m_CreateTimeX, m_timestring );
    }
    return m_CreateTimeX;
}

void Poi::SetCreateTime( wxDateTime dt )
{
    m_CreateTimeX = dt;
}


void Poi::SetName(const wxString & name)
{
    m_MarkName = name;
    CalculateNameExtents();
}

void Poi::CalculateNameExtents( void )
{
    if( m_pMarkFont ) {
        wxScreenDC dc;

        dc.SetFont( *m_pMarkFont );
        m_NameExtents = dc.GetTextExtent( m_MarkName );
    } else
        m_NameExtents = wxSize( 0, 0 );

}

/*
#ifdef ocpnUSE_GL
void Poi::DrawGL( ViewPort &vp, OCPNRegion &region )
{
    if( !m_bIsVisible )
    return;

    //    Optimization, especially apparent on tracks in normal cases
    if( m_IconName == _T("empty") && !m_bShowName && !m_bPtIsSelected ) return;

    if(m_wpBBox.GetValid() &&
       vp.chart_scale == m_wpBBox_chart_scale &&
       vp.rotation == m_wpBBox_rotation) {
         see if this waypoint can intersect with bounding box
        LLBBox vpBBox = vp.GetBBox();
        if( vpBBox.IntersectOut( m_wpBBox ) ) {
             try with vp crossing IDL
            if(vpBBox.GetMinX() < -180 && vpBBox.GetMaxX() > -180) {
                wxPoint2DDouble xlate( -360., 0. );
                wxBoundingBox test_box2 = m_wpBBox;
                test_box2.Translate( xlate );
                if( vp.GetBBox().IntersectOut( test_box2 ) )
                    return;
            } else 
            if(vpBBox.GetMinX() < 180 && vpBBox.GetMaxX() > 180) {
                wxPoint2DDouble xlate( 360., 0. );
                wxBoundingBox test_box2 = m_wpBBox;
                test_box2.Translate( xlate );
                if( vp.GetBBox().IntersectOut( test_box2 ) )
                    return;
            } else
                return;
        }
    }

    wxPoint r;
    wxRect hilitebox;
    unsigned char transparency = 100;

    cc1->GetCanvasPointPix( m_lat, m_lon, &r );

//    Substitue icon?
    wxBitmap *pbm;
    if( ( m_bIsActive ) && ( m_IconName != _T("mob") ) )
        pbm = pPoiMan->GetIconBitmap(  _T ( "activepoint" ) );
    else
        pbm = m_pbmIcon;

    int sx2 = pbm->GetWidth() / 2;
    int sy2 = pbm->GetHeight() / 2;

//    Calculate the mark drawing extents
    wxRect r1( r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2 );           // the bitmap extents

    wxRect r3 = r1;
    if( m_bShowName ) {
        if( !m_pMarkFont ) {
            m_pMarkFont = FontMgr::Get().GetFont( _( "Marks" ) );
            m_FontColor = FontMgr::Get().GetFontColor( _( "Marks" ) );
            CalculateNameExtents();
        }

        if( m_pMarkFont ) {
            wxRect r2( r.x + m_NameLocationOffsetX, r.y + m_NameLocationOffsetY,
                       m_NameExtents.x, m_NameExtents.y );
            r3.Union( r2 );
        }
    }

    hilitebox = r3;
    hilitebox.x -= r.x;
    hilitebox.y -= r.y;
    hilitebox.Inflate( 2 );  mouse poop?

     update bounding box
    if(!m_wpBBox.GetValid() || vp.chart_scale != m_wpBBox_chart_scale || vp.rotation != m_wpBBox_rotation) {
        double lat1, lon1, lat2, lon2;
        cc1->GetCanvasPixPoint(r.x+hilitebox.x, r.y+hilitebox.y+hilitebox.height, lat1, lon1);
        cc1->GetCanvasPixPoint(r.x+hilitebox.x+hilitebox.width, r.y+hilitebox.y, lat2, lon2);

        m_wpBBox.SetMin(lon1, lat1);
        m_wpBBox.SetMax(lon2, lat2);
        m_wpBBox_chart_scale = vp.chart_scale;
        m_wpBBox_rotation = vp.rotation;
    }

    if(region.Contains(r3) == wxOutRegion)
        return;

    ocpnDC dc;

    //  Highlite any selected point
    if( m_bPtIsSelected ) {
        wxPen *pen;
        if( m_bBlink )
            pen = g_pRouteMan->GetActiveRoutePointPen();
        else
            pen = g_pRouteMan->GetRoutePointPen();
        
        AlphaBlending( dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, 0.0,
                       pen->GetColour(), transparency );
    }
    
    bool bDrawHL = false;

    if( m_bBlink && ( gFrame->nBlinkerTick & 1 ) ) bDrawHL = true;

    if( ( !bDrawHL ) && ( NULL != m_pbmIcon ) ) {
        int glw, glh;
        unsigned int IconTexture = pPoiMan->GetIconTexture( pbm, glw, glh );
        
        glBindTexture(GL_TEXTURE_2D, IconTexture);
        
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        
        glColor3f(1, 1, 1);
        
        int x = r1.x, y = r1.y, w = r1.width, h = r1.height;
        float u = (float)w/glw, v = (float)h/glh;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(x, y);
        glTexCoord2f(u, 0); glVertex2f(x+w, y);
        glTexCoord2f(u, v); glVertex2f(x+w, y+h);
        glTexCoord2f(0, v); glVertex2f(x, y+h);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }

    if( m_bShowName && m_pMarkFont ) {
        int w = m_NameExtents.x, h = m_NameExtents.y;
        if(!m_iTextTexture && w && h) {
            wxBitmap tbm(w, h);  render text on dc
            wxMemoryDC dc;
            dc.SelectObject( tbm );               
            dc.SetBackground( wxBrush( *wxBLACK ) );
            dc.Clear();
            dc.SetFont( *m_pMarkFont );
            dc.SetTextForeground( *wxWHITE );
            dc.DrawText( m_MarkName, 0, 0);
            dc.SelectObject( wxNullBitmap );
            
             make alpha texture for text
            wxImage image = tbm.ConvertToImage();
            unsigned char *d = image.GetData();
            unsigned char *e = new unsigned char[w * h];
            for( int p = 0; p < w*h; p++)
                e[p] = d[3*p + 0];
            
             create texture for rendered text
            glGenTextures(1, &m_iTextTexture);
            glBindTexture(GL_TEXTURE_2D, m_iTextTexture);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

            m_iTextTextureWidth = NextPow2(w);
            m_iTextTextureHeight = NextPow2(h);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, m_iTextTextureWidth, m_iTextTextureHeight,
                         0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                            GL_ALPHA, GL_UNSIGNED_BYTE, e);
            delete [] e;
        }

        if(m_iTextTexture) {
             draw texture with text
            glBindTexture(GL_TEXTURE_2D, m_iTextTexture);
            
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
            glColor3ub(m_FontColor.Red(), m_FontColor.Green(), m_FontColor.Blue());
            
            int x = r.x + m_NameLocationOffsetX, y = r.y + m_NameLocationOffsetY;
            float u = (float)w/m_iTextTextureWidth, v = (float)h/m_iTextTextureHeight;
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(x, y);
            glTexCoord2f(u, 0); glVertex2f(x+w, y);
            glTexCoord2f(u, v); glVertex2f(x+w, y+h);
            glTexCoord2f(0, v); glVertex2f(x, y+h);
            glEnd();
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        }
    }
    
    if( m_bBlink ) g_blink_rect = CurrentRect_in_DC;               // also save for global blinker
    
    //    This will be useful for fast icon redraws
    CurrentRect_in_DC.x = r.x + hilitebox.x;
    CurrentRect_in_DC.y = r.y + hilitebox.y;
    CurrentRect_in_DC.width = hilitebox.width;
    CurrentRect_in_DC.height = hilitebox.height;

    if( m_bBlink ) g_blink_rect = CurrentRect_in_DC;               // also save for global blinker
}
#endif
*/

void Poi::SetPosition( double lat, double lon )
{
    m_lat = lat;
    m_lon = lon;
}

bool Poi::IsSame( Poi *pOtherRP )
{
    bool IsSame = false;

    if( this->m_MarkName == pOtherRP->m_MarkName ) {
        if( fabs( this->m_lat - pOtherRP->m_lat ) < 1.e-6
                && fabs( this->m_lon - pOtherRP->m_lon ) < 1.e-6 ) IsSame = true;
    }
    return IsSame;
}

// ------------------------------------------------------  from navutil.cpp
// This function parses a string containing a GPX time representation
// and returns a wxDateTime containing the UTC corresponding to the
// input. The function return value is a pointer past the last valid
// character parsed (if successful) or NULL (if the string is invalid).
//
// Valid GPX time strings are in ISO 8601 format as follows:
//
//   [-]<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>Z|(+|-<hh>:<mm>)
//
// For example, 2010-10-30T14:34:56Z and 2010-10-30T14:34:56-04:00
// are the same time. The first is UTC and the second is EDT.

const wxChar *Poi::ParseGPXDateTime( wxDateTime &dt, const wxChar *datetime )
{
    long sign, hrs_west, mins_west;
    const wxChar *end;

    // Skip any leading whitespace
    while( isspace( *datetime ) )
        datetime++;

    // Skip (and ignore) leading hyphen
    if( *datetime == wxT('-') ) datetime++;

    // Parse and validate ISO 8601 date/time string
    if( ( end = dt.ParseFormat( datetime, wxT("%Y-%m-%dT%T") ) ) != NULL ) {

        // Invalid date/time
        if( *end == 0 ) return NULL;

        // ParseFormat outputs in UTC if the controlling
        // wxDateTime class instance has not been initialized.

        // Date/time followed by UTC time zone flag, so we are done
        else
            if( *end == wxT('Z') ) {
                end++;
                return end;
            }

            // Date/time followed by given number of hrs/mins west of UTC
            else
                if( *end == wxT('+') || *end == wxT('-') ) {

                    // Save direction from UTC
                    if( *end == wxT('+') ) sign = 1;
                    else
                        sign = -1;
                    end++;

                    // Parse hrs west of UTC
                    if( isdigit( *end ) && isdigit( *( end + 1 ) ) && *( end + 2 ) == wxT(':') ) {

                        // Extract and validate hrs west of UTC
                        wxString( end ).ToLong( &hrs_west );
                        if( hrs_west > 12 ) return NULL;
                        end += 3;

                        // Parse mins west of UTC
                        if( isdigit( *end ) && isdigit( *( end + 1 ) ) ) {

                            // Extract and validate mins west of UTC
                            wxChar mins[3];
                            mins[0] = *end;
                            mins[1] = *( end + 1 );
                            mins[2] = 0;
                            wxString( mins ).ToLong( &mins_west );
                            if( mins_west > 59 ) return NULL;

                            // Apply correction
                            dt -= sign * wxTimeSpan( hrs_west, mins_west, 0, 0 );
                            return end + 2;
                        } else
                            // Missing mins digits
                            return NULL;
                    } else
                        // Missing hrs digits or colon
                        return NULL;
                } else
                    // Unknown field after date/time (not UTC, not hrs/mins
                    //  west of UTC)
                    return NULL;
    } else
        // Invalid ISO 8601 date/time
        return NULL;
}
