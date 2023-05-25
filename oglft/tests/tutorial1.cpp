/*
 * tutorial1.cpp: Tutorial for the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: tutorial1.cpp,v 1.5 2003/10/01 14:08:49 allen Exp $
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

// Declare a Face variable of the desired style
OGLFT::Monochrome* monochrome;

void init ( const char* filename )
{
  // Create a new face given the font filename and a size

  monochrome = new OGLFT::Monochrome( filename, 36 );

  // Always check to make sure the face was properly constructed

  if ( monochrome == 0 || !monochrome->isValid() ) {
    std::cerr << "Could not construct face from " << filename << std::endl;
    return;
  }

  // Set the face color to red

  monochrome->setForegroundColor( 1., 0., 0. );

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
  monochrome->draw( 0., 250., "Hello, World!" );
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
  glutInitDisplayMode( GLUT_RGB ); // Note: OGLFT really only works in RGB mode
  glutCreateWindow( argv[0] );

  init( argv[1] );

  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutMainLoop();

  return 0;
}
