/*
 * tutorial2.cpp: Tutorial for the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: tutorial2.cpp,v 1.5 2003/10/01 14:09:12 allen Exp $
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
using namespace std;

#include <GL/glut.h>
#include <config.h>
#include <OGLFT.h> // Note: this will depend on where you've installed OGLFT

#define USE_BITMAP_FACE

// Declare a Face variable of the desired style
#if defined( USE_BITMAP_FACE )
OGLFT::Monochrome* face;
#else
OGLFT::Filled* face;
#endif

void init ( const char* filename )
{
  // Create a new face given the font filename and a size

#if defined( USE_BITMAP_FACE )
  face = new OGLFT::Monochrome( filename, 36 );
#else
  face = new OGLFT::Filled( filename, 36 );
#endif

  // Always check to make sure the face was properly constructed

  if ( face == 0 || !face->isValid() ) {
    std::cerr << "Could not construct face from " << filename << std::endl;
    return;
  }

  // Set the face color to red

  face->setForegroundColor( 1., 0., 0. );

  // Use centered justification

  face->setHorizontalJustification( OGLFT::Face::CENTER );

  // For the raster styles, it is essential that the pixel store
  // unpacking alignment be set to 1

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  // Set the window's background color

  glClearColor( .75, .75, .75, 1. );
}

static void display ( void )
{
  // First clear the window ...
  glClear( GL_COLOR_BUFFER_BIT );
  // ... then draw the string
  face->draw( 250., 250., "Hello, World!" );

  glutSwapBuffers();
}

static void reshape ( int width, int height )
{
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, width, 0, height, -1, 1 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

static void idle ( void )
{
  // Retrieve the current value of the string's rotation and increment
  // it by 4 degrees

  face->setStringRotation( face->stringRotation() + 4 );

  // Too fast even without acceleration
  struct timespec request = { 0, 40000000 };
  nanosleep( &request, 0 );

  glutPostRedisplay();
}

int main ( int argc, char* argv[] )
{
  // Check to be sure the user specified something as a font file name

  if ( argc != 2 ) {
    std::cerr << "usage: " << argv[0] << " fontfile" << std::endl;
    return 1;
  }

  // Standard GLUT setup commands

  glutInit( &argc, argv );
  glutInitWindowSize( 500, 500 );
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE ); // Note: OGLFT really only works in RGB mode
  glutCreateWindow( argv[0] );

  init( argv[1] );

  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutIdleFunc( idle );
  glutMainLoop();

  return 0;
}
