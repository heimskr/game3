/*
 * demo2.cpp: Second Demo of the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: demo2.cpp,v 1.7 2003/10/01 14:04:49 allen Exp $
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

#include <stdio.h>

#include <GL/glut.h>

#include <config.h>
#include <OGLFT.h>

#include FT_MULTIPLE_MASTERS_H

static const char* USAGE = " fontfile";

static FT_Library library;
static FT_Face ft_face;
static FT_Multi_Master master_info;
static FT_Long axis_averages[T1_MAX_MM_AXIS];
#if 0
static OGLFT::Filled* oglft_face;
#else
static OGLFT::Monochrome* oglft_face;
#endif
static int viewport_width;
static int viewport_height;

static void init ( int argc, char* argv[] )
{
  std::cout << glGetString( GL_VENDOR ) << " " << glGetString( GL_RENDERER ) << " "
	    << glGetString( GL_VERSION ) << std::endl;

  library = OGLFT::Library::instance();

  FT_Error error;

  error = FT_New_Face( library, argv[1], 0, &ft_face );

  if ( error != 0 ) {
    std::cerr << "Could not create a font from file \"" << argv[1] << "\""
	      << std::endl;
    exit( 1 );
  }

  error = FT_Get_Multi_Master( ft_face, &master_info );

  if ( error != 0 ) {
    std::cerr << "Font file \"" << argv[1]
	      << "\" does not contain a multi master font" << std::endl;
    exit( 1 );
  }

  for ( unsigned int i = 0; i < master_info.num_axis; i++ )
    axis_averages[i] = ( master_info.axis[i].minimum +
			 master_info.axis[i].maximum ) / 2;

  FT_Set_MM_Design_Coordinates( ft_face, master_info.num_axis, axis_averages );

#if 0
  oglft_face = new OGLFT::Filled( ft_face, 14 );
#else
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#if defined( GL_RASTER_POSITION_UNCLIPPED_IBM )
  glEnable( GL_RASTER_POSITION_UNCLIPPED_IBM );
#endif
  oglft_face = new OGLFT::Monochrome( ft_face, 14 );
#endif
  oglft_face->setHorizontalJustification( OGLFT::Face::LEFT );
  oglft_face->setForegroundColor( 1., 0., 0. );
  oglft_face->setCompileMode( OGLFT::Face::IMMEDIATE );
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
}

static void display ( void )
{
  const int BUFSIZE = 128;
  char buffer[BUFSIZE];

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPushMatrix();

  snprintf( buffer, sizeof(buffer), "There are %d axes", master_info.num_axis );

  GLfloat y = 480.;

  oglft_face->draw( 0., y, buffer );

  for ( unsigned int i = 0; i < master_info.num_axis; i++ ) {
    FT_Set_MM_Design_Coordinates( ft_face, master_info.num_axis, axis_averages );
    snprintf( buffer, sizeof(buffer), "%s: min: %ld max: %ld",
	      master_info.axis[i].name,
	      master_info.axis[i].minimum, master_info.axis[i].maximum );

    y -= 20.;
    oglft_face->draw( 0., y, buffer );

    FT_Long axis_average = axis_averages[i];
    FT_Long d_axis = (master_info.axis[i].maximum - master_info.axis[i].minimum)/4;
    axis_averages[i] = master_info.axis[i].minimum;

    for ( int j = 0; j <= 4; j++ ) {
      FT_Set_MM_Design_Coordinates( ft_face, master_info.num_axis, axis_averages );
      snprintf( buffer, sizeof(buffer), "   Style at axis = %d\n", axis_averages[i] );
      y -= 20.;
      oglft_face->draw( 0., y, buffer );

      axis_averages[i] += d_axis;
    }

    axis_averages[i] = axis_average;
  }

  glPopMatrix();
  glutSwapBuffers();
}

static void key ( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 'q':
  case 27:
    exit( 0 );
  default:
    return;
  }

  glutPostRedisplay();
}

static void idle ( void )
{
  glutPostRedisplay();
}

static void done ( void )
{
}

int main ( int argc, char* argv[] )
{
  if ( argc != 2 ) {
    std::cerr << argv[0] << USAGE << std::endl;
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
