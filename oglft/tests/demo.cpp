/*
 * demo.cpp: Demo of the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: demo.cpp,v 1.4 2002/07/11 20:58:38 allen Exp $
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
#include <unistd.h>
#include <cstdlib>
#include <iostream>
using namespace std;

#include <GL/glut.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <OGLFT.h>

static const char* USAGE = " fontfile";

static const char* commands = "a/A: char rotate X, s/S: char rotate Y, d/D: char rotate Z, f/F: string rotate, r: reset all";
static const char* text = "The quick brown fox jumps over a lazy dog.";
static const float point_size = 16;

static OGLFT::Monochrome* commands_face;
static OGLFT::Monochrome* monochrome_face;
static OGLFT::Grayscale* grayscale_face;
static OGLFT::Translucent* translucent_face;
static OGLFT::Outline* outline_face;
static OGLFT::Filled* filled_face;
#ifndef OGLFT_NO_SOLID
static OGLFT::Solid* solid_face;
#else
static OGLFT::Monochrome* solid_face;
#endif
static OGLFT::MonochromeTexture* monochrome_texture_face;
static OGLFT::GrayscaleTexture* grayscale_texture_face;
static OGLFT::TranslucentTexture* translucent_texture_face;


static float dy;
static int viewport_width;
static int viewport_height;
static int x_rot, y_rot, z_rot;

static void init ( int argc, char* argv[] )
{
  cout << glGetString( GL_VENDOR ) << " " << glGetString( GL_RENDERER ) << " "
       << glGetString( GL_VERSION ) << endl;

  commands_face = new OGLFT::Monochrome( argv[1], point_size / 2. );
  commands_face->setHorizontalJustification( OGLFT::Face::CENTER );

  monochrome_face = new OGLFT::Monochrome( argv[1], point_size );
  monochrome_face->setHorizontalJustification( OGLFT::Face::CENTER );
  monochrome_face->setForegroundColor( 1., 0., 0., 1. );

  if ( !monochrome_face->isValid() ) {
    cerr << "failed to open face. exiting." << endl;
    exit( 1 );
  }

  grayscale_face = new OGLFT::Grayscale( argv[1], point_size );
  grayscale_face->setHorizontalJustification( OGLFT::Face::CENTER );
  grayscale_face->setForegroundColor( 0., 0., .5, 1. );
  grayscale_face->setBackgroundColor( 0., 1., 1., 1. );

  translucent_face = new OGLFT::Translucent( argv[1], point_size );
  translucent_face->setHorizontalJustification( OGLFT::Face::CENTER );
  translucent_face->setForegroundColor( 0., .5, 0., 1. );

  outline_face = new OGLFT::Outline( argv[1], point_size );
  outline_face->setHorizontalJustification( OGLFT::Face::CENTER );
  outline_face->setForegroundColor( 1., 1., 0., 1. );

  filled_face = new OGLFT::Filled( argv[1], point_size );
  filled_face->setHorizontalJustification( OGLFT::Face::CENTER );
  filled_face->setForegroundColor( .5, 0., 1., 1. );

#ifndef OGLFT_NO_SOLID
  solid_face = new OGLFT::Solid( argv[1], point_size );
  solid_face->setDepth( 10. );
  solid_face->setCharacterRotationX( 25. );
  solid_face->setCharacterRotationY( 25. );
  solid_face->setTessellationSteps( 3 );
#else
  solid_face = new OGLFT::Monochrome( argv[1], point_size );
#endif
  solid_face->setHorizontalJustification( OGLFT::Face::CENTER );
  solid_face->setForegroundColor( 1., .5, 0., 1. );

  monochrome_texture_face = new OGLFT::MonochromeTexture( argv[1], point_size );
  monochrome_texture_face->setHorizontalJustification( OGLFT::Face::CENTER );
  monochrome_texture_face->setForegroundColor( 0., .5, .75, 1. );

  grayscale_texture_face = new OGLFT::GrayscaleTexture( argv[1], point_size );
  grayscale_texture_face->setHorizontalJustification( OGLFT::Face::CENTER );
  grayscale_texture_face->setForegroundColor( 0.9, .65, .9, 1. );
  grayscale_texture_face->setBackgroundColor( 0.5, .5, .75, 0.3 );

  translucent_texture_face = new OGLFT::TranslucentTexture( argv[1], point_size );
  translucent_texture_face->setHorizontalJustification( OGLFT::Face::CENTER );
  translucent_texture_face->setForegroundColor( 0.75, 1., .75, 1. );

  // Set various general parameters which don't affect performance (yet).
  glClearColor( .75, .75, .75, 1. );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
  glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  glEnable( GL_LIGHT0 );
  glDisable( GL_DITHER );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
}

static void reshape ( int width, int height )
{
  viewport_width = width;
  viewport_height = height;

  glViewport( 0, 0, viewport_width, viewport_height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, viewport_width, 0, viewport_height, -100, 100 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  dy = viewport_height / ( 9 + 1 );
}

static void reset ( void )
{
  monochrome_face->setCharacterRotationZ( 0 );
  monochrome_face->setStringRotation( 0 );

  grayscale_face->setCharacterRotationZ( 0 );
  grayscale_face->setStringRotation( 0 );

  translucent_face->setCharacterRotationZ( 0 );
  translucent_face->setStringRotation( 0 );

  outline_face->setCharacterRotationX( 0 );
  outline_face->setCharacterRotationY( 0 );
  outline_face->setCharacterRotationZ( 0 );
  outline_face->setStringRotation( 0 );

  filled_face->setCharacterRotationX( 0 );
  filled_face->setCharacterRotationY( 0 );
  filled_face->setCharacterRotationZ( 0 );
  filled_face->setStringRotation( 0 );

#ifndef OGLFT_NO_SOLID
  solid_face->setCharacterRotationX( 25. );
  solid_face->setCharacterRotationY( 25. );
#endif
  solid_face->setCharacterRotationZ( 0 );
  solid_face->setStringRotation( 0 );

  monochrome_texture_face->setCharacterRotationX( 0 );
  monochrome_texture_face->setCharacterRotationY( 0 );
  monochrome_texture_face->setCharacterRotationZ( 0 );
  monochrome_texture_face->setStringRotation( 0 );

  grayscale_texture_face->setCharacterRotationX( 0 );
  grayscale_texture_face->setCharacterRotationY( 0 );
  grayscale_texture_face->setCharacterRotationZ( 0 );
  grayscale_texture_face->setStringRotation( 0 );

  translucent_texture_face->setCharacterRotationX( 0 );
  translucent_texture_face->setCharacterRotationY( 0 );
  translucent_texture_face->setCharacterRotationZ( 0 );
  translucent_texture_face->setStringRotation( 0 );

  glViewport( 0, 0, viewport_width, viewport_height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, viewport_width, 0, viewport_height, -100, 100 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

static void char_rotate_x ( float dx )
{
  outline_face->setCharacterRotationX( outline_face->characterRotationX()+dx );
  filled_face->setCharacterRotationX( filled_face->characterRotationX()+dx );
#ifndef OGLFT_NO_SOLID
  solid_face->setCharacterRotationX( solid_face->characterRotationX()+dx );
#endif
  monochrome_texture_face->setCharacterRotationX(
				 monochrome_texture_face->characterRotationX()+dx );
  grayscale_texture_face->setCharacterRotationX(
				grayscale_texture_face->characterRotationX()+dx );
  translucent_texture_face->setCharacterRotationX(
			  translucent_texture_face->characterRotationX()+dx );
}

static void char_rotate_y ( float dy )
{
  outline_face->setCharacterRotationY( outline_face->characterRotationY()+dy );
  filled_face->setCharacterRotationY( filled_face->characterRotationY()+dy );
#ifndef OGLFT_NO_SOLID
  solid_face->setCharacterRotationY( solid_face->characterRotationY()+dy );
#endif
  monochrome_texture_face->setCharacterRotationY(
				 monochrome_texture_face->characterRotationY()+dy );
  grayscale_texture_face->setCharacterRotationY(
				grayscale_texture_face->characterRotationY()+dy );
  translucent_texture_face->setCharacterRotationY(
			  translucent_texture_face->characterRotationY()+dy );
}

static void char_rotate_z ( float dz )
{
  monochrome_face->setCharacterRotationZ( monochrome_face->characterRotationZ()+dz );
  grayscale_face->setCharacterRotationZ( grayscale_face->characterRotationZ()+dz );
  translucent_face->setCharacterRotationZ( translucent_face->characterRotationZ()+dz );

  outline_face->setCharacterRotationZ( outline_face->characterRotationZ()+dz );
  filled_face->setCharacterRotationZ( filled_face->characterRotationZ()+dz );
  solid_face->setCharacterRotationZ( solid_face->characterRotationZ()+dz );

  monochrome_texture_face->setCharacterRotationZ( monochrome_texture_face->characterRotationZ()+dz );
  grayscale_texture_face->setCharacterRotationZ( grayscale_texture_face->characterRotationZ()+dz );
  translucent_texture_face->setCharacterRotationZ( translucent_texture_face->characterRotationZ()+dz );
}

static void string_rotate ( float dz )
{

  monochrome_face->setStringRotation( monochrome_face->stringRotation()+dz );
  grayscale_face->setStringRotation( grayscale_face->stringRotation()+dz );
  translucent_face->setStringRotation( translucent_face->stringRotation()+dz );

  outline_face->setStringRotation( outline_face->stringRotation()+dz );
  filled_face->setStringRotation( filled_face->stringRotation()+dz );
  solid_face->setStringRotation( solid_face->stringRotation()+dz );

  monochrome_texture_face->setStringRotation( monochrome_texture_face->stringRotation()+dz );
  grayscale_texture_face->setStringRotation( grayscale_texture_face->stringRotation()+dz );
  translucent_texture_face->setStringRotation( translucent_texture_face->stringRotation()+dz );
}

static void display ( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPushMatrix();

  // Draw everything centered
  glTranslatef( viewport_width/2., 0., 0. );

  commands_face->draw( 0., 0., commands );

  glTranslatef( 0., dy, 0. );
  monochrome_face->draw( 0., 0., text );

  glTranslatef( 0., dy, 0. );
  grayscale_face->draw( 0., 0., text );

  glEnable( GL_BLEND );
  glTranslatef( 0., dy, 0. );
  translucent_face->draw( 0., 0., text );
  glDisable( GL_BLEND );

  glTranslatef( 0., dy, 0. );
  outline_face->draw( 0., 0., text );

  glTranslatef( 0., dy, 0. );
  filled_face->draw( 0., 0., text );

  glTranslatef( 0., dy, 0. );
#ifndef OGLFT_NO_SOLID
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_COLOR_MATERIAL );
  solid_face->draw( 0., 0., text );
  glDisable( GL_COLOR_MATERIAL );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
#else
  solid_face->draw( 0., 0., "<Note: Solid face not available in library>" );
#endif

  glEnable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );
  glEnable( GL_DEPTH_TEST );
  glTranslatef( 0., dy, 0. );
  monochrome_texture_face->draw( 0., 0., text );

  glTranslatef( 0., dy, 0. );
  grayscale_texture_face->draw( 0., 0., text );

  glTranslatef( 0., dy, 0. );
  translucent_texture_face->draw( 0., 0., text );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_BLEND );
  glDisable( GL_TEXTURE_2D );

  glPopMatrix();
  glutSwapBuffers();
}

static void idle ( void )
{
  glutPostRedisplay();
}

static void key ( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 'a':
    char_rotate_x( -4 ); break;
  case 'A':
    char_rotate_x( 4 ); break;
  case 's':
    char_rotate_y( -4 ); break;
  case 'S':
    char_rotate_y( 4 ); break;
  case 'd':
    char_rotate_z( -4 ); break;
  case 'D':
    char_rotate_z( 4 ); break;
  case 'f':
    string_rotate( -4 ); break;
  case 'F':
    string_rotate( 4 ); break;
  case 'r': case 'R':
    reset(); break;
  case 27:
    exit( 0 );
  default:
    return;
  }

  glutPostRedisplay();
}

static void done ( void )
{
  delete monochrome_face;
  delete grayscale_face;
  delete translucent_face;
  delete outline_face;
  delete filled_face;
  delete solid_face;
  delete monochrome_texture_face;
  delete grayscale_texture_face;
  delete translucent_texture_face;
}

int main ( int argc, char* argv[] )
{
  if ( argc != 2 ) {
    cerr << argv[0] << USAGE << endl;
    return 1;
  }

  glutInit( &argc, argv );
  glutInitWindowSize( 500, 500 );
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutCreateWindow( argv[0] );

  init( argc, argv );

  atexit( done );

  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( key );
#if 0
  glutIdleFunc( idle );
#endif
  glutMainLoop();

  return 0;
}
