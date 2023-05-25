/* -*- c++ -*-
 * speedtest.h: Header for Performance test for the OGLFT library
 * Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
 * $Id: speedtest.h,v 1.2 2002/02/04 19:01:17 allen Exp $
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
#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#include <stdlib.h>

#include <qgl.h>
#include <qqueue.h>
#include <qtimer.h>

// Little animation vignettes. The function calls essentially follow
// those of the Qt OpenGL widget and are called at the corresponding times.

struct Vignette {
  virtual ~Vignette ( void ) {}
  virtual unsigned int frame_count ( void ) = 0;
  virtual void init ( void ) = 0;
  virtual void view ( GLdouble /*left*/, GLdouble /*right*/,
		      GLdouble /*bottom*/, GLdouble /*top*/ )
  {}
  virtual void reset ( void )
  {}
  virtual bool input ( QKeyEvent* e )
  {
    if ( e->key() == Qt::Key_Escape ) exit( 0 );
    return false;
  }
  virtual void draw ( int frame_number ) = 0;
  virtual void finish ( void ) = 0;
};

// Yet another OpenGL view widget.

class CharacterView : public QGLWidget {
Q_OBJECT
  GLsizei window_width_, window_height_;
  GLdouble view_width_, view_height_;
  GLdouble view_left_, view_right_, view_bottom_, view_top_;
  GLdouble rot_x_, rot_y_, rot_z_;

  QTimer redraw_timer_;
  QTimer performance_timer_;

  unsigned int frame_counter_;
  unsigned int counter_snapshot_;
  unsigned int animation_frame_count_;
  unsigned int iteration_counter_;
  unsigned int maximum_iterations_;

  static const unsigned int PERFORMANCE_SAMPLE_RATE_HZ = 1;
  static const unsigned int MAXIMUM_ITERATIONS = 4;

  QQueue<Vignette> vignettes;

protected slots:
  void redraw ( void );
  void measure_performance ( void );

public:
  CharacterView ( const char* text, const char* fontfile,
		  QWidget* parent = 0, const char* name = 0 );
protected:

  void initializeGL ( void );
  void resizeGL ( int w, int h );
  void paintGL ( void );
  void keyPressEvent ( QKeyEvent* e );
  void resetView ( void );
};

#endif /* SPEEDTEST_H */
