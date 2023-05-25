/*
 * demo4.cpp: Fourth Demo of the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: demo3.cpp,v 1.4 2002/03/26 12:48:43 allen Exp $
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
     cerr << "Could not construct face from " << filename << endl;
     return;
  }

  // Set the face color to red

  monochrome->setForegroundColor( 1., 0., 0. );
  //  monochrome->setHorizontalJustification( OGLFT::Face::CENTER );

  // For the raster styles, it is essential that the pixel store
  // unpacking alignment be set to 1

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  // Set the window's background color

  glClearColor( .75, .75, .75, 1. );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );

  GLfloat material[4] = { 0., 0., 1., 1. };
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material );
}

static GLint viewport[4];
static GLdouble modelview[16];
static GLdouble projection[16];

static void reshape ( int width, int height )
{
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glGetIntegerv( GL_VIEWPORT, viewport );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
}

static void display ( void )
{
  // First clear the window ...
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  OGLFT::BBox bbox = monochrome->measure( "gHelloj!" );
  OGLFT::BBox bbox_raw = monochrome->measureRaw( "gHelloj!" );

  GLdouble x0, y0, z0;
  gluUnProject( 0., 0., 0., modelview, projection, viewport, &x0, &y0, &z0 );

  GLdouble x1, y1, z1;
  gluUnProject( bbox_raw.x_min_, bbox_raw.y_min_, 0.,
		modelview, projection, viewport, &x1, &y1, &z1 );

  GLdouble x2, y2, z2;
  gluUnProject( bbox_raw.x_max_, bbox_raw.y_min_, 0.,
		modelview, projection, viewport, &x2, &y2, &z2 );

  GLdouble x3, y3, z3;
  gluUnProject( bbox_raw.x_max_, bbox_raw.y_max_, 0.,
		modelview, projection, viewport, &x3, &y3, &z3 );

  GLdouble x4, y4, z4;
  gluUnProject( bbox_raw.x_min_, bbox_raw.y_max_, 0.,
		modelview, projection, viewport, &x4, &y4, &z4 );

  glColor3f( 0., .5, 0. );
  glPushMatrix();
  glTranslated( .25-x0, 0.25-y0, 0.25-z0 );
  glBegin( GL_QUADS );
  glVertex3d( x1, y1, z1 );
  glVertex3d( x2, y2, z2 );
  glVertex3d( x3, y3, z3 );
  glVertex3d( x4, y4, z4 );
  glEnd();
  glPopMatrix();

  // ... then draw the string
  monochrome->draw( 0.25, 0.25, 0.25, "gHelloj!" );
#if 0
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
  glutSolidCube( .5 );
  glDisable( GL_LIGHT0 );
  glDisable( GL_LIGHTING );
#endif
  glutSwapBuffers();
}

enum { NONE, SPIN, PAN } action = NONE;
int old_x, old_y;

static void mouse ( int button, int state, int x, int y )
{
  if ( state == GLUT_UP )
    action = NONE;
  else if ( button == GLUT_LEFT_BUTTON ) {
    action = SPIN;
    old_x = x;
    old_y = y;
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  }
  else if ( button == GLUT_MIDDLE_BUTTON ) {
    action = PAN;
    old_x = x;
    old_y = y;
  }
  else if ( button == GLUT_RIGHT_BUTTON ) {
    glLoadIdentity();
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glutPostRedisplay();
  }
}

static void motion ( int x, int y )
{
  int mdx = x - old_x, mdy = y - old_y;

  switch ( action ) {
  case SPIN: {
    GLfloat position[] = {0, 0, 1, 0};
    glLoadIdentity();
    glLightfv( GL_LIGHT0, GL_POSITION, position );
    glLoadMatrixd( modelview );
    glRotated( mdy, modelview[0], modelview[4], modelview[8] );
    glRotated( mdx, modelview[1], modelview[5], modelview[9] );
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  }
  break;
  case PAN: {
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );
    GLdouble dx = 2. * (GLdouble)mdx / viewport[2];
    GLdouble dy = 2. * (GLdouble)(-mdy) / viewport[3];
    glTranslated( dx, dy, 0 );
  }
  }

  old_x = x; old_y = y;

  glutPostRedisplay();
}

int main ( int argc, char* argv[] )
{
  // Check to be sure the user specified something as a font file name

  if ( argc != 2 ) {
    cerr << "usage: " << argv[0] << " fontfile" << endl;
    return 1;
  }

  // Standard GLUT setup commands

  glutInit( &argc, argv );
  glutInitWindowSize( 500, 500 );
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutCreateWindow( argv[0] );

  init( argv[1] );

  glutReshapeFunc( reshape );
  glutDisplayFunc( display );
  glutMouseFunc( mouse );
  glutMotionFunc( motion );
  glutMainLoop();

  return 0;
}
