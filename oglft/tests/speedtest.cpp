/*
 * speedtest.cpp: Performance test for the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: speedtest.cpp,v 1.6 2003/10/01 14:08:30 allen Exp $
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
#include <cstdlib>
using namespace std;

#include <qapplication.h>

#include <config.h>
#include <OGLFT.h>

#include "speedtest.h"


class Vignette0 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Face* face_;

public:
  Vignette0 ( const char* text, const char* fontfile ) : text_( text )
  {
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 0: Color and depth buffer clearing only" << std::endl;
  }

  void draw ( int frame_number )
  {
  }

  void finish ( void )
  {
  }
};

class Vignette1 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Monochrome* face_;

public:
  Vignette1 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Monochrome( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 1: MONOCHROME: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0., text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette2 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Monochrome* face_;

public:
  Vignette2 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Monochrome( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 2: MONOCHROME: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0., text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette3 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Monochrome* face_;

  GLuint dlist_;

public:
  Vignette3 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Monochrome( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setAdvance( false );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 3: MONOCHROME: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glRasterPos2f( 0., 0. );
    glCallList( dlist_ );
  }

  void finish ( void )
  {
  }
};

class Vignette4 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Grayscale* face_;

  GLuint dlist_;

public:
  Vignette4 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Grayscale( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 4: GRAYSCALE: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette5 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Grayscale* face_;

public:
  Vignette5 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Grayscale( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 5: GRAYSCALE: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette6 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Grayscale* face_;

  GLuint dlist_;

public:
  Vignette6 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Grayscale( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 6: GRAYSCALE: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glRasterPos2f( 0., 0. );
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette7 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Translucent* face_;

  GLuint dlist_;

public:
  Vignette7 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Translucent( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 7: TRANSLUCENT: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_BLEND );
  }
};

class Vignette8 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Translucent* face_;

public:
  Vignette8 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Translucent( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 8: TRANSLUCENT: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_BLEND );
  }
};

class Vignette9 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Translucent* face_;

  GLuint dlist_;

public:
  Vignette9 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Translucent( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 9: TRANSLUCENT: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
    glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glRasterPos2f( 0., 0. );
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_BLEND );
  }
};

class Vignette10 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Outline* face_;

  GLuint dlist_;

public:
  Vignette10 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Outline( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 10: OUTLINE: immediate drawing: " << text_ << std::endl;
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette11 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Outline* face_;

public:
  Vignette11 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Outline( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 11: OUTLINE: cached glyphs: " << text_ << std::endl;
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette12 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Outline* face_;

  GLuint dlist_;

public:
  Vignette12 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Outline( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
    face_->setAdvance( false );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 12: OUTLINE: display list: " << text_ << std::endl;
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    //    glPushMatrix();
    glCallList( dlist_ );
    //    glPopMatrix();
  }

  void finish ( void )
  {
  }
};

class Vignette13 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Filled* face_;

  GLuint dlist_;

public:
  Vignette13 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 13: FILLED: immediate drawing: " << text_ << std::endl;
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette14 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Filled* face_;

public:
  Vignette14 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 14: FILLED: cached glyphs: " << text_ << std::endl;
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
  }
};

class Vignette15 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Filled* face_;

  GLuint dlist_;

public:
  Vignette15 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Filled( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 15: FILLED: display list: " << text_ << std::endl;
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
  }
};
#ifndef OGLFT_NO_SOLID
class Vignette16 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Solid* face_;

  GLuint dlist_;

public:
  Vignette16 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Solid( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 16: SOLID: immediate drawing: " << text_ << std::endl;
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_LIGHTING );
  }
};

class Vignette17 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Solid* face_;

public:
  Vignette17 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Solid( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 17: SOLID: cached glyphs: " << text_ << std::endl;
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_LIGHTING );
  }
};

class Vignette18 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::Solid* face_;

  GLuint dlist_;

public:
  Vignette18 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::Solid( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 18: SOLID: display list: " << text_ << std::endl;
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_LIGHTING );
  }
};
#endif // OGLFT_NO_SOLID
class Vignette19 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::MonochromeTexture* face_;

  GLuint dlist_;

public:
  Vignette19 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::MonochromeTexture( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 19: MONOCHROME TEXTURE: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette20 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::MonochromeTexture* face_;

public:
  Vignette20 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::MonochromeTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 20: MONOCHROME TEXTURE: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette21 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::MonochromeTexture* face_;

  GLuint dlist_;

public:
  Vignette21 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::MonochromeTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 21: MONOCHROME TEXTURE: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette22 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::GrayscaleTexture* face_;

  GLuint dlist_;

public:
  Vignette22 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::GrayscaleTexture( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 22: GRAYSCALE TEXTURE: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette23 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::GrayscaleTexture* face_;

public:
  Vignette23 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::GrayscaleTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 23: GRAYSCALE TEXTURE: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette24 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::GrayscaleTexture* face_;

  GLuint dlist_;

public:
  Vignette24 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::GrayscaleTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 24: GRAYSCALE TEXTURE: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
  }
};

class Vignette25 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::TranslucentTexture* face_;

  GLuint dlist_;

public:
  Vignette25 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::TranslucentTexture( fontfile, 20, 75 );
    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 25: TRANSLUCENT TEXTURE: immediate drawing: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
  }
};

class Vignette26 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::TranslucentTexture* face_;

public:
  Vignette26 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::TranslucentTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 26: TRANSLUCENT TEXTURE: cached glyphs: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  void draw ( int frame_number )
  {
    face_->draw( 0., 0.,  text_ );
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
  }
};

class Vignette27 : public Vignette {

  static const unsigned int FRAME_COUNT = 128; // Not really

  const char* text_;
  OGLFT::TranslucentTexture* face_;

  GLuint dlist_;

public:
  Vignette27 ( const char* text, const char* fontfile ) : text_( text )
  {
    face_ = new OGLFT::TranslucentTexture( fontfile, 20, 75 );
    //    face_->setCompileMode( OGLFT::Face::IMMEDIATE );
  }

  unsigned int frame_count ( void ) { return FRAME_COUNT; }

  void init ( void )
  {
    std::cout << "Vignette 27: TRANSLUCENT TEXTURE: display list: " << text_ << std::endl;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    dlist_ = face_->compile( text_ );
  }

  void draw ( int frame_number )
  {
    glPushMatrix();
    glCallList( dlist_ );
    glPopMatrix();
  }

  void finish ( void )
  {
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
  }
};

CharacterView::CharacterView ( const char* text, const char* fontfile,
			       QWidget* parent, const char* name )
  : QGLWidget( parent, name )
{
  vignettes.enqueue( new Vignette0( text, fontfile ) );
  vignettes.enqueue( new Vignette1( text, fontfile ) );
  vignettes.enqueue( new Vignette2( text, fontfile ) );
  vignettes.enqueue( new Vignette3( text, fontfile ) );
  vignettes.enqueue( new Vignette4( text, fontfile ) );
  vignettes.enqueue( new Vignette5( text, fontfile ) );
  vignettes.enqueue( new Vignette6( text, fontfile ) );
  vignettes.enqueue( new Vignette7( text, fontfile ) );
  vignettes.enqueue( new Vignette8( text, fontfile ) );
  vignettes.enqueue( new Vignette9( text, fontfile ) );
  vignettes.enqueue( new Vignette10( text, fontfile ) );
  vignettes.enqueue( new Vignette11( text, fontfile ) );
  vignettes.enqueue( new Vignette12( text, fontfile ) );
  vignettes.enqueue( new Vignette13( text, fontfile ) );
  vignettes.enqueue( new Vignette14( text, fontfile ) );
  vignettes.enqueue( new Vignette15( text, fontfile ) );
#ifndef OGLFT_NO_SOLID
  vignettes.enqueue( new Vignette16( text, fontfile ) );
  vignettes.enqueue( new Vignette17( text, fontfile ) );
  vignettes.enqueue( new Vignette18( text, fontfile ) );
#endif
  vignettes.enqueue( new Vignette19( text, fontfile ) );
  vignettes.enqueue( new Vignette20( text, fontfile ) );
  vignettes.enqueue( new Vignette21( text, fontfile ) );
  vignettes.enqueue( new Vignette22( text, fontfile ) );
  vignettes.enqueue( new Vignette23( text, fontfile ) );
  vignettes.enqueue( new Vignette24( text, fontfile ) );
  vignettes.enqueue( new Vignette25( text, fontfile ) );
  vignettes.enqueue( new Vignette26( text, fontfile ) );
  vignettes.enqueue( new Vignette27( text, fontfile ) );

  frame_counter_ = 0;
  counter_snapshot_ = 0;
  iteration_counter_ = MAXIMUM_ITERATIONS;

  animation_frame_count_ = vignettes.current()->frame_count();

  connect( &redraw_timer_, SIGNAL(timeout()), SLOT(redraw()) );
  connect( &performance_timer_, SIGNAL(timeout()), SLOT(measure_performance()) );

  redraw_timer_.start( 0 );
  performance_timer_.start( PERFORMANCE_SAMPLE_RATE_HZ * 1000 );

}

void CharacterView::redraw ( void ) 
{
  frame_counter_++;
  updateGL();
}

void CharacterView::measure_performance ( void )
{
  int delta_count = frame_counter_ - counter_snapshot_;

  std::cout << delta_count << " FPS" << std::endl;

  counter_snapshot_ = frame_counter_;

  iteration_counter_--;

  if ( iteration_counter_ == 0 ) {
    vignettes.current()->finish();
    vignettes.dequeue();

    if ( !vignettes.isEmpty() ) {
      frame_counter_ = 0;
      counter_snapshot_ = 0;
      iteration_counter_ = MAXIMUM_ITERATIONS;

      vignettes.current()->init();
      animation_frame_count_ = vignettes.current()->frame_count();
    }
    else {
      // Evidently, events may be processed during exiting, so...
      redraw_timer_.stop();
      performance_timer_.stop();
      qApp->exit( 0 );
    }
  }
}

void CharacterView::initializeGL ( void )
{
  std::cout << glGetString( GL_VENDOR ) << " " << glGetString( GL_RENDERER ) << " "
       << glGetString( GL_VERSION ) << std::endl;

  glClearColor( 0.75, 0.75, 0.75, 1. );

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

  rot_x_ = rot_y_ = rot_z_ = 0.;

  resetView();
}

void CharacterView::paintGL ( void )
{

  glClear( GL_COLOR_BUFFER_BIT );

  vignettes.current()->draw( frame_counter_ % animation_frame_count_ );
}

void CharacterView::keyPressEvent ( QKeyEvent* e )
{
  static const GLdouble ZOOM = 1.0625;
  static const GLdouble PAN = 0.002;
  static const GLdouble ROTATOR = 1.;
  GLdouble center;
  bool redraw = true;
  GLdouble speed = 1.0;

  if ( e->state() & ShiftButton )
    speed *= 2.0;
  if ( e->state() & ControlButton )
    speed *= 2.0;
  if ( e->state() & AltButton )
    speed *= 2.0;

  switch ( e->key() ) {
  case Key_PageUp:
    view_width_ = (GLdouble)view_width_ / (speed * ZOOM);
    center = ( view_left_ + view_right_ ) / 2.;
    view_left_ = center - view_width_ / 2.;
    view_right_ = center + view_width_ / 2.;

    view_height_ = (GLdouble)view_height_ / (speed * ZOOM);
    center = ( view_bottom_ + view_top_ ) / 2.;
    view_bottom_ = center - view_height_ / 2.;
    view_top_ = center + view_height_ / 2.;
    break;
  case Key_PageDown:
    view_width_ = (GLdouble)view_width_ * (speed * ZOOM);
    center = ( view_left_ + view_right_ ) / 2.;
    view_left_ = center - view_width_ / 2.;
    view_right_ = center + view_width_ / 2.;

    view_height_ = (GLdouble)view_height_ * (speed * ZOOM);
    center = ( view_bottom_ + view_top_ ) / 2.;
    view_bottom_ = center - view_height_ / 2.;
    view_top_ = center + view_height_ / 2.;
    break;
  case Key_Up:
    view_bottom_ += (speed * PAN) * view_height_;
    view_top_ = view_bottom_ + view_height_;
    break;
  case Key_Down:
    view_bottom_ -= (speed * PAN) * view_height_;
    view_top_ = view_bottom_ + view_height_;
    break;
  case Key_Left:
    view_left_ -= (speed * PAN) * view_width_;
    view_right_ = view_left_ + view_width_;
    break;
  case Key_Right:
    view_left_ += (speed * PAN) * view_width_;
    view_right_ = view_left_ + view_width_;
    break;
  case Key_Q:
  case Key_Escape:
    qApp->exit( 0 );
  case Key_R:
    view_width_ = window_width_;
    view_height_ = window_height_;

    view_left_ = -view_width_ / 2.;
    view_right_ = view_width_ / 2.;
    view_bottom_ = -view_height_ / 2.;
    view_top_ = view_height_ / 2.;

    rot_x_ = rot_y_ = rot_z_ = 0;

    vignettes.current()->reset();
    break;
  case Key_X:
    rot_x_ += speed * ROTATOR;
    break;
  case Key_Y:
    rot_y_ += speed * ROTATOR;
    break;
  case Key_Z:
    rot_z_ += speed * ROTATOR;
    break;
  default:
    redraw = vignettes.current()->input( e );
  }
  if ( redraw ) {
    resetView();
    updateGL();
  }
}

void CharacterView::resetView ( void )
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
#if 1
  glOrtho( view_left_, view_right_, view_bottom_, view_top_, -250., 250. );
#else
  glFrustum( view_left_, view_right_, view_bottom_, view_top_,
	     10., 100. );
#endif
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glRotatef( rot_x_, 1., 0., 0. );
  glRotatef( rot_y_, 0., 1., 0. );
  glRotatef( rot_z_, 0., 0., 1. );

  glTranslatef( -window_width_ / 2., -window_height_ / 2., 0. );
  glTranslatef( 0., 0., -10. );

  vignettes.current()->view( view_left_, view_right_, view_bottom_, view_top_ );
}

int main ( int argc, char* argv[] )
{

  QApplication app( argc, argv );

  if ( argc != 3 ) {
    std::cerr << "usage: " << argv[0] << " string fontfile" << std::endl;
    return 1;
  }

  CharacterView cv ( argv[1], argv[2] );
  cv.resize( 500, 500 );

  app.setMainWidget( &cv );
  cv.show();

  return app.exec();
}
