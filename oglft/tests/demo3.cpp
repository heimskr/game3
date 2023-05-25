/*
 * demo3.cpp: Third Demo of the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: demo3.cpp,v 1.6 2003/10/01 14:05:08 allen Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <iostream>
#include <algorithm>
using namespace std;

#include <qapplication.h>
#include <qimage.h>

#include <config.h>
#include <OGLFT.h>
#include <vignette.h>

#include <Demo3UnicodeExample.h>
#include <Demo3UnicodeExample2.h>

class Vignette0 : public Vignette {

  static const unsigned int FRAME_COUNT = 33;
  static const unsigned int FRAME_RATE = 16;

  const char* text_;
  OGLFT::Filled* face_;

public:
  Vignette0 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 288, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
  }

  ~Vignette0 ( void )
  {
    //    std::cout << "destructing Vignette 0" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 0" << std::endl;
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();
    glTranslatef( 0., 0.,
		  - 1. - ( FRAME_COUNT - frame_number ) );
    face_->draw( 0., 0., text_ );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette1 : public Vignette {

  static const unsigned int FRAME_COUNT = 49;
  static const unsigned int FRAME_RATE = 16;

  const char* text_;
  OGLFT::Filled* face_;

public:
  Vignette1 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 288, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
  }

  ~Vignette1 ( void )
  {
    //    std::cout << "destructing Vignette 1" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 1" << std::endl;
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();
    glTranslatef( 0., 0.,
		  - 1. - ( FRAME_COUNT - frame_number ) );
    glRotatef( frame_number * 15., 0., 0., 1. );
    face_->draw( 0., 0., text_ );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette2 : public Vignette {

  static const unsigned int FRAME_COUNT = 60;
  static const unsigned int FRAME_RATE = 12;

  const char* text_;
  OGLFT::Filled* face_;

  GLfloat x_;

public:
  Vignette2 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 32, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::RIGHT );
    face_->setCharacterRotationZ( 30. );

    x_ = -250.;
  }

  ~Vignette2 ( void )
  {
    //    std::cout << "destructing Vignette 2" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 2" << std::endl;
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();

    if ( frame_number <= 48 ) {
      x_ += 10.;
      face_->setCharacterRotationZ( face_->characterRotationZ() - 30. );
    }
    else
      x_ = 250.;

    glTranslatef( x_, 0., 0. );

    face_->draw( 0., 0., text_ );

    glPopMatrix();
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void finish ( void )
  {
  }
};

class Vignette3 : public Vignette {

  static const unsigned int FRAME_COUNT = 36;
  static const unsigned int FRAME_RATE = 16;

  const char* text_;
  OGLFT::Filled* face_;

public:
  Vignette3 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 32, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->setCharacterRotationY( 90. );
  }

  ~Vignette3 ( void )
  {
    //    std::cout << "destructing Vignette 3" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 3" << std::endl;
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();
    glTranslatef( 0., 0.,
		  - 1. - ( FRAME_COUNT - frame_number ) );
    face_->setCharacterRotationY( face_->characterRotationY() + 10. );
    face_->draw( 0., 0., text_ );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette4 : public Vignette {

  static const unsigned int FRAME_COUNT = 82;
  static const unsigned int FRAME_RATE = 10;

  const char* text_;
#ifndef OGLFT_NO_SOLID
  OGLFT::Solid* face_;
#else
  OGLFT::Filled* face_;
#endif
  OGLFT::Translucent* annotation_;

  QImage* bg00b;
  QImage* bg01;
  QImage* bg10b;
  QImage* bg11;

  GLuint bg_textures_[6];

public:
  Vignette4 ( const char* text, const char* fontfile ) : text_( text )
  {
#ifndef OGLFT_NO_SOLID
    face_ = new OGLFT::Solid( fontfile, 120, 75 );
    face_->setDepth( 24. );
#else
    face_ = new OGLFT::Filled( fontfile, 120, 75 );
#endif
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setForegroundColor( 1., 1., 0. );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->setVerticalJustification( OGLFT::Face::MIDDLE );
    face_->setCharacterRotationY( 0. );
    face_->setCharacterRotationZ( 0. );

    annotation_ = new OGLFT::Translucent( fontfile, 10, 75 );
    annotation_->setForegroundColor( 1., 1., 0., 0.5 );
    annotation_->setHorizontalJustification( OGLFT::Face::RIGHT );
    annotation_->setVerticalJustification( OGLFT::Face::BOTTOM );

    bg00b = new QImage( "background00b.png" );
    bg01 = new QImage( "background01.png" );
    bg10b = new QImage( "background10b.png" );
    bg11 = new QImage( "background11.png" );
  }

  ~Vignette4 ( void )
  {
    //    std::cout << "destructing Vignette 4" << std::endl;
    glDeleteTextures( 6, bg_textures_ );
    delete bg00b;
    delete bg01;
    delete bg10b;
    delete bg11;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 4" << std::endl;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glGenTextures( 6, bg_textures_ );

    glBindTexture( GL_TEXTURE_2D, bg_textures_[1] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bg01->width(), bg01->height(),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, bg01->bits() );

    glBindTexture( GL_TEXTURE_2D, bg_textures_[3] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bg11->width(), bg11->height(),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, bg11->bits() );

    glBindTexture( GL_TEXTURE_2D, bg_textures_[4] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bg00b->width(), bg00b->height(),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, bg00b->bits() );

    glBindTexture( GL_TEXTURE_2D, bg_textures_[5] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bg10b->width(), bg10b->height(),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, bg10b->bits() );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_LIGHT0 );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );

    GLint viewport[4];
    GLdouble matrix[16];

    glGetIntegerv( GL_VIEWPORT, viewport );
    glGetDoublev( GL_PROJECTION_MATRIX, matrix );
    glGetDoublev( GL_MODELVIEW_MATRIX, matrix );
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();

    GLubyte alpha = 255;

    if ( frame_number < FRAME_COUNT )
      alpha =  (GLubyte)( 255. * (GLfloat)frame_number/(FRAME_COUNT) );

    glEnable( GL_BLEND );

    glBegin( GL_QUADS );
    glColor4ub( 128, 164, 212, alpha );
    glVertex2f( -250., 0. );
    glVertex2f( 250., 0. );
    glColor4ub( 101, 142, 198, alpha );
    glVertex2f( 250., 250. );
    glVertex2f( -250., 250. );
    glEnd();

    glDisable( GL_BLEND );

    glEnable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_COLOR_MATERIAL );

    GLfloat y;

    if ( frame_number <= 72 ) {
      face_->setCharacterRotationX( face_->characterRotationX() + 25. );
      face_->setCharacterRotationY( face_->characterRotationY() + 25. );
#if 0
      face_->setCharacterRotationZ( face_->characterRotationZ() - 12.5 );
#endif
      y = 3.5*frame_number - 55.;
    }
    else
      y = 3.5*72 - 55.;

    face_->draw( 0., y, text_ );

    if ( frame_number < FRAME_COUNT ) {
      // Fade in the background
      GLdouble f = (GLfloat)frame_number/(FRAME_COUNT);

      glColor4f( f, f, f, 1. );
    }
    else
      glColor4f( 1., 1., 1., 1. );

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_COLOR_MATERIAL );

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );

    glBindTexture( GL_TEXTURE_2D, bg_textures_[1] );

    glBegin( GL_QUADS );
    glTexCoord2f( 0., 1. ); glVertex2f( -250., -250. );
    glTexCoord2f( 1., 1. ); glVertex2f( 0., -250. );
    glTexCoord2f( 1., 0. ); glVertex2f( 0., 0. );
    glTexCoord2f( 0., 0. ); glVertex2f( -250., 0. );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, bg_textures_[3] );

    glBegin( GL_QUADS );
    glTexCoord2f( 0., 1. ); glVertex2f( 0., -250. );
    glTexCoord2f( 1., 1. ); glVertex2f( 250., -250. );
    glTexCoord2f( 1., 0. ); glVertex2f( 250., 0. );
    glTexCoord2f( 0., 0. ); glVertex2f( 0., 0. );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, bg_textures_[4] );

    glBegin( GL_QUADS );
    glTexCoord2f( 0., 1. ); glVertex2f( -250., 0. );
    glTexCoord2f( 1., 1. ); glVertex2f( 0., 0. );
    glTexCoord2f( 1., 0. ); glVertex2f( 0., 250. );
    glTexCoord2f( 0., 0. ); glVertex2f( -250., 250. );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, bg_textures_[5] );

    glBegin( GL_QUADS );
    glTexCoord2f( 0., 1. ); glVertex2f( 0., 0. );
    glTexCoord2f( 1., 1. ); glVertex2f( 250., 0. );
    glTexCoord2f( 1., 0. ); glVertex2f( 250., 250. );
    glTexCoord2f( 0., 0. ); glVertex2f( 0., 250. );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );

    glEnable( GL_BLEND );

    annotation_->draw( 250., -250., "Hoodoos @ Bryce Canyon National Park, Utah, USA" );

    glDisable( GL_BLEND );

    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette5 : public Vignette {

  static const unsigned int FRAME_COUNT = 124;
  static const unsigned int FRAME_RATE = 8;

  const char* text_;
  OGLFT::Monochrome* monochrome_;
  OGLFT::Grayscale* grayscale_;
  OGLFT::Translucent* translucent_;

  OGLFT::Outline* outline_;
  OGLFT::Filled* filled_;
#ifndef OGLFT_NO_SOLID
  OGLFT::Solid* solid_;
#else
  OGLFT::Monochrome* solid_;
#endif

  OGLFT::MonochromeTexture* monochrome_texture_;
  OGLFT::GrayscaleTexture* grayscale_texture_;
  OGLFT::TranslucentTexture* translucent_texture_;

public:
  Vignette5 ( const char* text, const char* fontfile ) : text_( text )
  {
    int point_size = 20;

    monochrome_ = new OGLFT::Monochrome( fontfile, point_size, 75 );
    monochrome_->setHorizontalJustification( OGLFT::Face::CENTER );
    monochrome_->setForegroundColor( 1., 0., 0., 1. );

    grayscale_ = new OGLFT::Grayscale( fontfile, point_size, 75 );
    grayscale_->setHorizontalJustification( OGLFT::Face::CENTER );
    grayscale_->setForegroundColor( 0., 0., .5, 1. );
    grayscale_->setBackgroundColor( 0., 1., 1., 1. );

    translucent_ = new OGLFT::Translucent( fontfile, point_size, 75 );
    translucent_->setHorizontalJustification( OGLFT::Face::CENTER );
    translucent_->setForegroundColor( 0., .5, 0., 1. );

    outline_ = new OGLFT::Outline( fontfile, point_size, 75 );
    outline_->setForegroundColor( 1., 1., 0., 1. );
    outline_->setHorizontalJustification( OGLFT::Face::CENTER );

    filled_ = new OGLFT::Filled( fontfile, point_size, 75 );
    filled_->setForegroundColor( .5, 0., 1., 1. );
    filled_->setHorizontalJustification( OGLFT::Face::CENTER );
#ifndef OGLFT_NO_SOLID
    solid_ = new OGLFT::Solid( fontfile, point_size, 75 );
    solid_->setDepth( 10. );
    solid_->setCharacterRotationX( 25. );
    solid_->setCharacterRotationY( 25. );
    solid_->setTessellationSteps( 3 );
#else
    solid_ = new OGLFT::Monochrome( fontfile, point_size, 75 );
#endif
    solid_->setHorizontalJustification( OGLFT::Face::CENTER );
    solid_->setForegroundColor( 1., .5, 0., 1. );

    monochrome_texture_ = new OGLFT::MonochromeTexture( fontfile, point_size, 75 );
    monochrome_texture_->setHorizontalJustification( OGLFT::Face::CENTER );
    monochrome_texture_->setForegroundColor( 0., .5, .75, 1. );

    grayscale_texture_ = new OGLFT::GrayscaleTexture( fontfile, point_size, 75 );
    grayscale_texture_->setHorizontalJustification( OGLFT::Face::CENTER );
    grayscale_texture_->setForegroundColor( 0.9, .65, .9, 1. );
    grayscale_texture_->setBackgroundColor( 0.5, .5, .75, 0.3 );

    translucent_texture_ = new OGLFT::TranslucentTexture( fontfile, point_size, 75 );
    translucent_texture_->setHorizontalJustification( OGLFT::Face::CENTER );
    translucent_texture_->setForegroundColor( 0.75, 1., .75, 1. );
  }

  ~Vignette5 ( void )
  {
    //    std::cout << "destructing Vignette 5" << std::endl;
    delete monochrome_;
    delete grayscale_;
    delete translucent_;
    delete outline_;
    delete filled_;
    delete solid_;
    delete monochrome_texture_;
    delete grayscale_texture_;
    delete translucent_texture_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 5" << std::endl;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_LIGHT0 );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void draw ( unsigned int frame_number )
  {
    GLdouble y;

    glPushMatrix();
    if ( frame_number <= 12 )
      y = -240. + 490. * ( 12 - frame_number ) / 12;
    else
      y = -240.;
    monochrome_->draw( 0., y, "Draw text as monochrome bitmaps" );

    if ( frame_number >= 12 ) {
      if ( frame_number <= 24 )
	y = -190. + 440. * ( 24 - frame_number ) / 12;
      else
	y = -190.;
      grayscale_->draw( 0., y, "Draw text as antialiased, grayscale pixmaps" );
    }

    if ( frame_number >= 24 ) {
      if ( frame_number <= 36 )
	y = -140. + 390. * ( 36 - frame_number ) / 12;
      else
	y = -140.;
      glEnable( GL_BLEND );
      translucent_->draw( 0., y, "Draw text as antialiased, blended pixmaps" );
      glDisable( GL_BLEND );
    }

    if ( frame_number >= 36 ) {
      if ( frame_number <= 48 )
	y = -90. + 340. * ( 48 - frame_number ) / 12;
      else
	y = -90.;
      outline_->draw( 0., y, "Draw text as line segments" );
    }

    if ( frame_number >= 48 ) {
      if ( frame_number <= 60 )
	y = -40. + 290. * ( 60 - frame_number ) / 12;
      else
	y = -40.;
      filled_->draw( 0., y, "Draw text as filled polygons" );
    }

    if ( frame_number >= 60 ) {
      if ( frame_number <= 72 )
	y = 10. + 240. * ( 72 - frame_number ) / 12;
      else
	y = 10.;
#ifndef OGLFT_NO_SOLID
      glEnable( GL_LIGHTING );
      glEnable( GL_DEPTH_TEST );
      glEnable( GL_COLOR_MATERIAL );
      solid_->draw( 0., y, "Draw text as solid with GLE" );
      glDisable( GL_COLOR_MATERIAL );
      glDisable( GL_DEPTH_TEST );
      glDisable( GL_LIGHTING );
#else
      solid_->draw( 0., y, "<Solid not available in library>" );
#endif
    }

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );

    if ( frame_number >= 72 ) {
      if ( frame_number <= 84 )
	y = 60. + 190. * ( 84 - frame_number ) / 12;
      else
	y = 60.;
      monochrome_texture_->draw( 0., y, "Draw text as monochrome texture maps" );
    }

    if ( frame_number >= 84 ) {
      if ( frame_number <= 96 )
	y = 110. + 140. * ( 96 - frame_number ) / 12;
      else
	y = 110.;
      grayscale_texture_->draw( 0., y, "Draw text as antialiased, grayscale texture maps" );
    }

    if ( frame_number >= 96 ) {
      if ( frame_number <= 108 )
	y = 160. + 90. * ( 108 - frame_number ) / 12;
      else
	y = 160.;
      translucent_texture_->draw( 0., y, "Draw text as antialiased, blended texture maps" );
    }

    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );

    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette6 : public Vignette {

  static const unsigned int FRAME_COUNT = 48;
  static const unsigned int FRAME_RATE = 12;

  const char* text_;
  OGLFT::Filled* face_;

  class MyColorTess : public OGLFT::ColorTess {
  public:
    QColor hsv_;
    GLfloat colors_[4];
    int phase_;
    MyColorTess ()
    {
      hsv_.setHsv( 0, 255, 255 );
      colors_[OGLFT::R] = hsv_.red() / 255.;
      colors_[OGLFT::G] = hsv_.green() / 255.;
      colors_[OGLFT::B] = hsv_.blue() / 255.;
      colors_[OGLFT::A] = 1.;
      phase_ = 0;
    }
    void setPhase ( int phase ) { phase_ = phase; }
    int phase ( void ) const { return phase_; }
  };

  class MyColorTessVertical : public MyColorTess
  {
  public:
    GLfloat* color ( GLdouble* p )
    {
      int hue = (int)( 360. * p[OGLFT::Y] / 36. + phase_ ) % 360;
      if ( hue < 0 ) hue += 360;
      hsv_.setHsv( hue, 255, 255 );
      colors_[OGLFT::R] = hsv_.red() / 255.;
      colors_[OGLFT::G] = hsv_.green() / 255.;
      colors_[OGLFT::B] = hsv_.blue() / 255.;
      return colors_;
    }
  };

  class MyColorTessHorizontal : public MyColorTess
  {
  public:
    GLfloat* color ( GLdouble* p )
    {
      int hue = (int)( 360. * p[OGLFT::X] / 36. + phase_ ) % 360;
      if ( hue < 0 ) hue += 360;
      hsv_.setHsv( hue, 255, 255 );
      colors_[OGLFT::R] = hsv_.red() / 255.;
      colors_[OGLFT::G] = hsv_.green() / 255.;
      colors_[OGLFT::B] = hsv_.blue() / 255.;
      return colors_;
    }
  };

  MyColorTessVertical color_tess_v_;
  MyColorTessHorizontal color_tess_h_;

public:
  Vignette6 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 72, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->setVerticalJustification( OGLFT::Face::MIDDLE );
    face_->setTessellationSteps( 3 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  ~Vignette6 ( void )
  {
    //    std::cout << "destructing Vignette 6" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 6" << std::endl;
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void draw ( unsigned int /*frame_number*/ )
  {
    glPushMatrix();
    face_->setColorTess( &color_tess_h_ );
    face_->draw( 0., 72., "Apply a per" );
    face_->draw( 0., 0., "vertex color" );
    face_->draw( 0., -72., "function" );
    glPopMatrix();

    color_tess_h_.setPhase( color_tess_h_.phase() + 15 );
    color_tess_v_.setPhase( color_tess_v_.phase() + 15 );
  }

  void finish ( void )
  {
  }
};

class Vignette7 : public Vignette {

  static const unsigned int FRAME_COUNT = 48;
  static const unsigned int FRAME_RATE = 12;

  const char* text_;
  OGLFT::Filled* face_;
  QImage* image_;
  GLuint texture_;

  class MyTextureTess : public OGLFT::TextureTess {
    int phase_;
    GLfloat texCoords_[2];
  public:
    MyTextureTess ()
    {
      texCoords_[0] = texCoords_[1] = 0.;
      phase_ = 0;
    }
    GLfloat* texCoord ( GLdouble* p ) {
      texCoords_[0] = ( p[OGLFT::X] + phase_ ) / 18.;
      texCoords_[1] = ( p[OGLFT::Y] + phase_ ) / 18.;
      return texCoords_;
    }
    void setPhase ( int phase ) { phase_ = phase; }
    int phase ( void ) const { return phase_; }
  };

  MyTextureTess texture_tess_;

public:
  Vignette7 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 72, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->setVerticalJustification( OGLFT::Face::MIDDLE );
    face_->setTessellationSteps( 3 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setTextureTess( &texture_tess_ );

    image_ = new QImage( "texture.png" );
  }

  ~Vignette7 ( void )
  {
    //    std::cout << "destructing Vignette 7" << std::endl;
    glDeleteTextures( 1, &texture_ );
    delete image_;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 7" << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glGenTextures( 1, &texture_ );

    glBindTexture( GL_TEXTURE_2D, texture_ );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image_->width(), image_->height(),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, image_->bits() );

    glEnable( GL_TEXTURE_2D );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void draw ( unsigned int frame_number )
  {
    glPushMatrix();
    face_->draw( 0., 144., "Apply a" );
    face_->draw( 0., 72., "per vertex" );
    face_->draw( 0., 0., "texture" );
    face_->draw( 0., -72., "function" );
    texture_tess_.setPhase( frame_number );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette8 : public Vignette {

  static const unsigned int FRAME_COUNT = 48;
  static const unsigned int FRAME_RATE = 3;

  char* text_;
  unsigned int text_length_;
  OGLFT::Monochrome* face_;

  static const int N_SIZES = 8;
  static const int sizes_[N_SIZES];
public:
  Vignette8 ( const char* text, const char* fontfile )
  {
    face_ = new OGLFT::Monochrome( fontfile, 36, 75 );
    face_->setForegroundColor( 1., 0., 0. );
    text_ = strdup( text );
    text_length_ = strlen( text_ );
  }

  ~Vignette8 ( void )
  {
    //    std::cout << "destructing Vignette 8" << std::endl;
    delete face_;
    free( text_ );
  }

  unsigned int frame_count ( void ) { return text_length_ + 3; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 8" << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    glPushAttrib( GL_POLYGON_BIT );
  }

  void draw ( unsigned int frame_number )
  {
    char save_char;
    if ( frame_number < text_length_ ) {
      save_char = text_[frame_number];
      text_[frame_number] = '\0';
    }

    glPushMatrix();

    glTranslatef( -225., 250., 0. );

    for ( int i = 0; i < N_SIZES; i++ ) {
      glTranslatef( 0., -2. * sizes_[i], 0. );

      face_->setPointSize( sizes_[i] );

      OGLFT::BBox size = face_->measure( text_ );

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glColor3f( 1., 1., 1. );
      glRectf( size.x_min_, size.y_min_, size.x_max_, size.y_max_ );

      face_->draw( 0., 0., text_ );

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glColor3f( 0., 0., 1. );
      glRectf( size.x_min_, 0., size.x_max_, size.y_max_ );
    }

    if ( frame_number < text_length_ )
      text_[frame_number] = save_char;

    glPopMatrix();
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void finish ( void )
  {
    glPopAttrib();
  }
};

const int Vignette8::sizes_[8] = { 64, 48, 32, 24, 18, 12, 10, 6 };

class Vignette9 : public Vignette {

  static const unsigned int FRAME_COUNT = 32;
  static const unsigned int FRAME_RATE = 8;

  const char* text_;
  OGLFT::Filled* face_;

public:
  Vignette9 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 288, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->setCharacterRotationX( 12. );
  }

  ~Vignette9 ( void )
  {
    //    std::cout << "destructing Vignette 9" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 9" << std::endl;
  }

  void draw ( unsigned int /*frame_number*/ )
  {
    glPushMatrix();
    glTranslatef( 0., 0., -8. );
    face_->draw( 0., 0., text_ );
    face_->setCharacterRotationX( face_->characterRotationX() - 0.5 );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette10 : public Vignette {

  static const unsigned int FRAME_COUNT = 108;
  static const unsigned int FRAME_RATE = 8;

  const char* text_;
  unsigned int text_length_;
  OGLFT::Filled* face_;

  GLuint glyph_dl_;
  OGLFT::DisplayLists dlists;
  GLfloat x_;

public:
  Vignette10 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 24, 75 );
    face_->setForegroundColor( 0., .5, .75 );
    face_->setHorizontalJustification( OGLFT::Face::LEFT );

    text_length_ = strlen( text_ );

    x_ = 250.;
  }

  ~Vignette10 ( void )
  {
    //    std::cout << "destructing Vignette 10" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 10" << std::endl;

    glyph_dl_ = glGenLists( 2 * text_length_ );

    dlists.push_back( 0 );

    QColor hsv;

    for ( unsigned int i = 0; i < text_length_; i++ ) {
     float dy = 50. * ( + sin( M_PI + (i+1) * 4 * M_PI / text_length_ )
			- sin( M_PI + i * 4 * M_PI / text_length_ ) );

      hsv.setHsv( (int)(360. * i / text_length_), 255, 255 );

      glNewList( glyph_dl_ + i, GL_COMPILE );
      glTranslatef( 0., dy, 0. );
      glColor3ub( hsv.red(), hsv.green(), hsv.blue() );
      glEndList();

      dlists.push_back( glyph_dl_ + i );
    }

    for ( unsigned int i = 0; i < text_length_; i++ ) {
      float y = 50. * sin( M_PI - i * 4 * M_PI / text_length_ );

      hsv.setHsv( (int)(360. * (text_length_ - i) / text_length_), 255, 255 );

      glNewList( glyph_dl_ + text_length_ + i, GL_COMPILE );
      glTranslatef( 0., y, 0. );
      glColor3ub( hsv.red(), hsv.green(), hsv.blue() );
      glEndList();

      dlists.push_back( glyph_dl_ + text_length_ + i );
    }

    dlists[0] = dlists[text_length_];

    face_->setCharacterDisplayLists( dlists );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
  }

  void draw ( unsigned int /*frame_number*/ )
  {
    OGLFT::DLI first = face_->characterDisplayLists().begin() + 1;
    OGLFT::DLI next = first + text_length_ - 1;
    OGLFT::DLI last = first + text_length_;

    rotate( first, next, last );

    first = face_->characterDisplayLists().begin() + text_length_ + 1;
    next = first + 1;
    last = first + text_length_;

    rotate( first, next, last );

    face_->characterDisplayLists()[0] =
      face_->characterDisplayLists()[text_length_+1];

    face_->draw( x_, 0., text_ );

    x_ -= 10.;
  }

  void finish ( void )
  {
    glDeleteLists( glyph_dl_, 2 * text_length_ );
  }
};

class Vignette11 : public Vignette {

  static const unsigned int FRAME_COUNT = 10;
  static const unsigned int FRAME_RATE = 1;

  const QString text_;
  QString equation_;
  OGLFT::Translucent* face_;

public:
  Vignette11 ( const char* text, const char* fontfile ) : text_( text )
  {
    // First, open a face in the usual way.
    face_ = new OGLFT::Translucent( fontfile, 18, 75 );

    // Now, create a second face, in this case, using a built-in font.
    FT_Library library = OGLFT::Library::instance();
    FT_Face ft_face;
    int a = FT_New_Memory_Face( library, Demo3UnicodeExample_ttf,
			Demo3UnicodeExample_ttf_size, 0, &ft_face );
    cout << "The return value from new mem face is " << a << endl;
    cout << "Ft face is " << ft_face << endl;
    face_->addAuxiliaryFace( ft_face );

    face_->setForegroundColor( 0., .5, .75 );

    // Manually create the equation in UNICODE points
    equation_  = QChar( 0x2207 );//Nabla
    equation_ += QChar( 0x2219 );//Dot
    equation_ += QChar( 0x03a9 );//Omega
    equation_ += QChar( 0x03c8 );//psi
    equation_ += QChar( ' ' );
    equation_ += QChar( '+' );
    equation_ += QChar( ' ' );
    equation_ += QChar( 0x03c3 );//sigma
    equation_ += QChar( 0x03c8 );//psi
    equation_ += QChar( ' ' );
    equation_ += QChar( '=' );
    equation_ += QChar( ' ' );
    equation_ += QChar( 0x222b );//integral
    equation_ += QChar( 'd' );
    equation_ += QChar( 0x03a9 );//Omega
    equation_ += QChar( '\'' );
    equation_ += QChar( 0x03c3 );//sigma
    equation_ += QChar( '(' );
    equation_ += QChar( 0x03a9 );//Omega
    equation_ += QChar( '\'' );
    equation_ += QChar( 0x2192 );//right arrow
    equation_ += QChar( 0x03a9 );//Omega
    equation_ += QChar( ')' );
    equation_ += QChar( 0x03c8 );//psi
    equation_ += QChar( '(' );
    equation_ += QChar( 0x03a9 );//Omega
    equation_ += QChar( '\'' );
    equation_ += QChar( ')' );
  }

  ~Vignette11 ( void )
  {
    //    std::cout << "destructing Vignette 11" << std::endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 11" << std::endl;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
    glTranslated( left, 0, 0 );
  }

  void draw ( unsigned int frame_number )
  {
    face_->setHorizontalJustification( OGLFT::Face::LEFT );
    face_->draw( 2., 128., "This frame demonstrates two...no three...features:" );
    face_->draw( 2., 96., "1. Drawing a UNICODE string (using Qt's QString)." );
    face_->draw( 2., 64., "2. Combining two fonts to cover more UNICODE points." );
    face_->draw( 2., 32., "3. Embedding a font in the program." );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->draw( 250., 0., equation_ );
  }

  void finish ( void )
  {
    glDisable( GL_BLEND );
  }
};

class Vignette12 : public Vignette {

  static const unsigned int FRAME_COUNT = 10;
  static const unsigned int FRAME_RATE = 1;

  const QString text_;
  QString equation_;
#if 1
  OGLFT::Monochrome* face_;
#else
  OGLFT::Filled* face_;
#endif
public:
  Vignette12 ( const char* text, const char* fontfile ) : text_( text )
  {
    // First, open a face in the usual way.
#if 1
    face_ = new OGLFT::Monochrome( fontfile, 18, 75 );
#else
    face_ = new OGLFT::Filled( fontfile,  18, 75 );
#endif
    face_->setForegroundColor( 0., .5, .75 );

    // Now, create a second face, in this case, using a built-in font.
    FT_Library library = OGLFT::Library::instance();
    FT_Face ft_face;
    FT_New_Memory_Face( library, lCSymbols_ttf,
			lCSymbols_ttf_size, 0, &ft_face );

    face_->addAuxiliaryFace( ft_face );
  }

  ~Vignette12 ( void )
  {
    //    cout << "destructing Vignette 12" << endl;
    delete face_;
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  unsigned int frame_rate ( void ) { return FRAME_RATE; }

  void init ( void )
  {
    std::cout << "Vignette 12" << std::endl;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
  }

  void view ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top )
  {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, -250., 250. );
    glMatrixMode( GL_MODELVIEW );
    glTranslated( left, 0, 0 );
  }

  void draw ( unsigned int frame_number )
  {
    face_->setVerticalJustification( OGLFT::Face::TOP );
    face_->setHorizontalJustification( OGLFT::Face::CENTER );
    face_->draw( 250., 250., "Example of numeric formatting" );

    face_->setVerticalJustification( OGLFT::Face::BASELINE );
    face_->setHorizontalJustification( OGLFT::Face::ORIGIN );

    face_->draw( 0., 128., "%.6f", 100.123456789 );
    face_->draw( 0., 96., "%12.6e", 100.123456789 );
    face_->draw( 0., 64., "%12.6g", 100.123456789 );
    for ( int i = 1; i <= 8; ++i ) {
      for ( int j = 1; j <= i; j++ ) {
	double a = i + (double)j / ( 1 << i );

	OGLFT::BBox bbox = face_->measure( "%p\"", a );

	glPushMatrix();
	glTranslated( j*42, -i * 18, 0 );

	glColor3f( .5, .5, 0. );
	glRectd( bbox.x_min_, bbox.y_min_, bbox.x_max_, bbox.y_max_ );

	face_->draw( 0, 0, "%p\"", a );
	glPopMatrix();
      }
    }

    for ( int r = 0; r < 360; r += 45 ) {
      double sinr = sin( (double)r/180 * M_PI );
      double cosr = cos( (double)r/180 * M_PI );
      face_->setStringRotation( r );
      OGLFT::BBox bbox = face_->measure( "%p\"", 1.015625 );

      glPushMatrix();
      glTranslated( 250.+cosr*24, 96.+sinr*24, 0 );

      glColor3f( .5, .5, 0. );
      glRectd( bbox.x_min_, bbox.y_min_, bbox.x_max_, bbox.y_max_ );

      face_->draw( 0, 0, "%p\"", 1.015625 );
      glPopMatrix();
    }

    face_->setStringRotation( 0 );
  }

  void finish ( void )
  {
    glDisable( GL_BLEND );
  }
};

CharacterView::CharacterView ( bool flank_speed, const char* fontfile,
			       QWidget* parent, const char* name )
  : QGLWidget( parent, name ), flank_speed_( flank_speed )
{
  vignettes.enqueue( new Vignette0( "Welcome to OGLFT!", fontfile ) );
  vignettes.enqueue( new Vignette1( "The OpenGL/FreeType library", fontfile ) );
  vignettes.enqueue( new Vignette2( "Featuring a mind numbing", fontfile ) );
  vignettes.enqueue( new Vignette3( "collection of rendering options", fontfile ) );
  vignettes.enqueue( new Vignette9( "and other text effects.", fontfile ) );
  vignettes.enqueue( new Vignette5( "Sampler", fontfile ) );
  vignettes.enqueue( new Vignette8( "Measuring the text.", fontfile ) );
  vignettes.enqueue( new Vignette6( "Per vertex color", fontfile ) );
  vignettes.enqueue( new Vignette7( "Per vertex texture coord", fontfile ) );
  vignettes.enqueue( new Vignette10( "Each glyph can have its own display list", fontfile ) );
  vignettes.enqueue( new Vignette11( "QString Example", fontfile ) );
  vignettes.enqueue( new Vignette12( "Formatting Numbers", fontfile ) );
  vignettes.enqueue( new Vignette4( "OGLFT", fontfile ) );

  frame_counter_ = 0;
  counter_snapshot_ = 0;
  animation_frame_counter_ = 0;

  animation_frame_count_ = vignettes.current()->frame_count();
  animation_frame_rate_ = 1000 / vignettes.current()->frame_rate();

  connect( &redraw_timer_, SIGNAL(timeout()), SLOT(redraw()) );
  connect( &performance_timer_, SIGNAL(timeout()), SLOT(measure_performance()) );

  if ( flank_speed_ )
    redraw_timer_.start( 0 );
  else
    redraw_timer_.start( animation_frame_rate_ );

  performance_timer_.start( PERFORMANCE_SAMPLE_RATE_HZ * 1000 );
}

void CharacterView::redraw ( void ) 
{
  updateGL();

  frame_counter_++;
  animation_frame_counter_++;

  if ( animation_frame_counter_ == animation_frame_count_ ) {
    redraw_timer_.stop();
    vignettes.current()->finish();
    delete vignettes.dequeue();

    if ( !vignettes.isEmpty() ) {
      counter_snapshot_ = frame_counter_;
      animation_frame_counter_ = 0;

      vignettes.current()->init();
      resetView();
      animation_frame_count_ = vignettes.current()->frame_count();
      animation_frame_rate_ = 1000 / vignettes.current()->frame_rate();

      if ( flank_speed_ )
	redraw_timer_.start( 0 );
      else
	redraw_timer_.start( animation_frame_rate_ );
    }
    else {
      // Evidently, events may be processed during exiting, so...
      redraw_timer_.stop();
      performance_timer_.stop();
      qApp->exit( 0 );
    }
  }
}

void CharacterView::measure_performance ( void )
{
  int delta_count = frame_counter_ - counter_snapshot_;

  std::cout << delta_count << " FPS" << std::endl;

  counter_snapshot_ = frame_counter_;
}

void CharacterView::initializeGL ( void )
{
  std::cout << glGetString( GL_VENDOR ) << " " << glGetString( GL_RENDERER ) << " "
       << glGetString( GL_VERSION ) << std::endl;

  glClearColor( 0., 0., 0., 1. );

  // Let the first vignette do any initialization it wants (though this
  // should probably be called by the animation controller).

  vignettes.current()->init();
}

void CharacterView::resizeGL ( int w, int h )
{
  window_width_ = w;
  window_height_ = h;

  glViewport( 0, 0, window_width_, window_height_ );

  view_width_ = window_width_;
  view_height_ = window_height_;

  view_left_ = -view_width_ / 2.;
  view_right_ = view_width_ / 2.;
  view_bottom_ = -view_height_ / 2.;
  view_top_ = view_height_ / 2.;

  resetView();
}

void CharacterView::paintGL ( void )
{

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  vignettes.current()->draw( animation_frame_counter_ % animation_frame_count_ );
}

void CharacterView::keyPressEvent ( QKeyEvent* e )
{
  switch ( e->key() ) {
  case Key_Q:
  case Key_Escape:
    qApp->exit( 0 );
  }
}

void CharacterView::resetView ( void )
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glFrustum( view_left_, view_right_, view_bottom_, view_top_,
	     1., 100. );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  vignettes.current()->view( view_left_, view_right_, view_bottom_, view_top_ );
}

int main ( int argc, char* argv[] )
{

  QApplication app( argc, argv );

  if ( argc < 2 ) {
    std::cerr << "usage: " << argv[0] << " [-f] fontfile" << std::endl;
    std::cerr << " -f   ignore the animation timings and run as fast as possible"
	      << std::endl;
    return 1;
  }

  int argn = 1;
  bool flank_speed = false;

  if ( !strcmp( argv[argn], "-f" ) ) {
    flank_speed = true;
    argn++;
  }

  if ( argc <= argn ) {
    std::cerr << "usage: " << argv[0] << " [-f] fontfile" << std::endl;
    std::cerr << " -f   ignore the animation timings and run as fast as possible"
	      << std::endl;
    return 1;
  }

  // Test the supplied face to make sure it will work OK

  OGLFT::Monochrome* test_face = new OGLFT::Monochrome( argv[argn] );

  if ( !test_face->isValid() ) {
    std::cerr << "Freetype did not recognize \"" << argv[1] << "\" as a font file"
	      << std::endl;
    return 1;
  }

  delete test_face;

  CharacterView cv( flank_speed, argv[argn] );
  cv.resize( 500, 500 );

  app.setMainWidget( &cv );
  cv.show();

  return app.exec();
}
