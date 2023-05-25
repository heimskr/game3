/*
 * tutorial3.cpp: Tutorial for the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: tutorial3.cpp,v 1.5 2003/10/01 14:09:28 allen Exp $
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
#include <cmath>
#include <ctime>
#include <vector> // The STL vector
#include <algorithm> // The STL algorithms
using namespace std;

#include <GL/glut.h>
#include <config.h>
#include <OGLFT.h> // Note: this will depend on where you've installed OGLFT

// A Face variable of the desired style
#ifndef OGLFT_NO_SOLID
OGLFT::Solid* solid;
#else // If Solid is not defined, use Filled instead
OGLFT::Filled* solid;
#endif

// The Bounding Box for the string
OGLFT::BBox bbox;

// A vector of OpenGL display list names
OGLFT::DisplayLists dlists;

// A vector of displacements defining the ocean
struct vertex {
  float y;
  float nx, ny;
};
std::vector< vertex > ocean_vertices;

void init ( const char* filename )
{
  // Create a new face given the font filename and a size

#ifndef OGLFT_NO_SOLID
  solid = new OGLFT::Solid( filename, 36 );
#else
  solid = new OGLFT::Filled( filename, 36 );
#endif

  // Always check to make sure the face was properly constructed

  if ( solid == 0 || !solid->isValid() ) {
    std::cerr << "Could not construct face from " << filename << std::endl;
    return;
  }

  const float AMPLITUDE = 25.;
  GLuint dlist = glGenLists( 2*13 );

  // The per character display lists are executed before the glyph is
  // rendered; so, the first display list must contain an absolute
  // transformation, but the subsequent ones must contain relative
  // transformations. However, we need complete sets of both transformations,
  // starting with a place holder for the first (absolute) transformation

  dlists.push_back( 0 );

  // Next, generate a sequence of relative displacements

  for ( int i=0; i<13; i++ ) {
    float dy = AMPLITUDE * ( sin( (i+1) * 2 * M_PI / 13 ) - sin( i * 2 * M_PI / 13 ) );

    glNewList( dlist, GL_COMPILE );
    glTranslatef( 0., dy, 0. );
    glEndList();

    dlists.push_back( dlist );

    dlist++;
  }

  // Next, generate a sequence of absolute displacements

  for ( int i=0; i<13; i++ ) {
    float y = AMPLITUDE * sin( i * 2 * M_PI / 13 );

    glNewList( dlist, GL_COMPILE );
    glTranslatef( 0., y, 0. );
    glEndList();

    dlists.push_back( dlist );

    dlist++;
  }

  // Finally, copy the first absolute displacement into the first element
  // of the display list vector

  dlists[0] = dlists[13+1];

  // Use centered justification

  solid->setHorizontalJustification( OGLFT::Face::CENTER );
#ifndef OGLFT_NO_SOLID
  // Make the glyphs rather thick

  solid->setDepth( 10. );
#endif
  // Apply the per character display lists

  solid->setCharacterDisplayLists( dlists );

  // Get the size of the string before it is transformed

  bbox = solid->measure( "Hello, World!" );

  // Make it (sea) green

  solid->setForegroundColor( 143./255., 188./255., 143./255. );

  // Set the window's background color

  glClearColor( .5, .5, .5, 1. );

  // Build an "ocean" for the characters to float upon (a higher resolution
  // version of the absolute displacements which we'll cycle through)

  for ( int i = 0; i <= 52; i++ ) {
    float s = sin( i * 2 * M_PI / 52 );
    float c = cos( i * 2 * M_PI / 52 );
    vertex v;
    v.y = AMPLITUDE * s;
    v.nx = c;
    v.ny = s;
    ocean_vertices.push_back( v );
  }

  // Enable lighting and the depth test

  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
  glEnable( GL_COLOR_MATERIAL );
  glEnable( GL_DEPTH_TEST );
}

static int offset = 0;

static void display ( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPushMatrix();

  solid->draw( 250., 250., "Hello, World!" );

  glTranslatef( 254.- ( bbox.x_max_ + bbox.x_min_ ) / 2., 255., 0. );

  glBegin( GL_QUAD_STRIP );

  glColor3f( 0., 0., 1. );

  for ( int i=0; i<=52; i++ ) {
    float x = i * ( bbox.x_max_ - bbox.x_min_ ) / 52;

    glNormal3f( ocean_vertices[(i+offset)%53].nx,
		ocean_vertices[(i+offset)%53].ny,
		0 );

    glVertex3f( x, ocean_vertices[(i+offset)%53].y, -100. );
    glVertex3f( x, ocean_vertices[(i+offset)%53].y, 100. );
  }

  offset = offset < 48 ? offset+4 : 0;

  glEnd();

  glPopMatrix();
  glutSwapBuffers();
}

static void reshape ( int width, int height )
{
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, width, 0, height, -200, 200 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  // Rotate the model slightly out of the XY plane so it looks 3D
  // (note we rotate the model instead of the view so that the lighting
  // is fixed relative to the view instead of the model)

  glTranslatef( width/2, height/2, 0. );
  glRotatef( 25., 1., 0., 0. );
  glRotatef( 25., 0., 1., 0. );
  glTranslatef( -width/2, -height/2, 0. );
}

static void idle ( void )
{
  // Use the STL rotate algorithm to animate the transformation display lists.
  // First, rotate the lists containing the relative displacements

  OGLFT::DLI first = solid->characterDisplayLists().begin()+1;
  OGLFT::DLI next = first + 1;
  OGLFT::DLI last = first + 13;

  rotate( first, next, last );

  // Next, rotate the the lists containing the absolute displacements

  first = solid->characterDisplayLists().begin() + 13 + 1;
  next = first + 1;
  last = first + 13;

  rotate( first, next, last );

  // Finally, copy the current absolute displacement into the leading element

  solid->characterDisplayLists()[0] = solid->characterDisplayLists()[13+1];

  glutPostRedisplay();

  // Too fast even without acceleration
  struct timespec request = { 0, 80000000 };
  nanosleep( &request, 0 );
}

static void key ( unsigned char c, int x, int y )
{
  switch ( c ) {
  case 27:
    exit( 0 );
  }
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
  glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
  glutCreateWindow( argv[0] );

  init( argv[1] );

  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutKeyboardFunc( key );

  glutIdleFunc( idle );

  glutMainLoop();

  return 0;
}
