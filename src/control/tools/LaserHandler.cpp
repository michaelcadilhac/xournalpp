#include "LaserHandler.h"

#include <iostream>
#include <cmath>
#include <memory>

#include <gdk/gdk.h>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/shaperecognizer/ShapeRecognizerResult.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"
#include "undo/RecognizerUndoAction.h"

#include "config-features.h"

guint32 LaserHandler::lastStrokeTime;  // persist for next stroke

LaserHandler::LaserHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
  InputHandler(xournal, redrawable, page) {}

LaserHandler::~LaserHandler() {
  std::cout << "Killed" << std::endl;
}

void LaserHandler::draw(cairo_t* cr) {
  std::cout << "draw(" << this->x << ", " << this->y << "), en=" << this->enabled << std::endl;
  if (!this->enabled)
    return;
  cairo_save(cr);
  cairo_set_line_width(cr, LASER_RADIUS * 2);
  cairo_arc(cr, this->x, this->y, LASER_RADIUS, 0.0, 2 * M_PI);
  const double r = ((this->color >> 24) & 0xFF) / 255.0;
  const double g = ((this->color >> 16) & 0xFF) / 255.0;
  const double b = ((this->color >> 8) & 0xFF) / 255.0;
  const double a = (this->color & 0xFF) / 255.0;
  cairo_set_source_rgba(cr, r, g, b, a);
  cairo_fill(cr);
  cairo_restore(cr);
}

auto LaserHandler::onKeyEvent(GdkEventKey* event) -> bool { return false; }

auto LaserHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
  std::cout << "OnMotion(" << pos.x << ", " << pos.y << ")" << std::endl;
  repaint ();
  this->x = pos.x;
  this->y = pos.y;
  repaint ();
  return true;
}

void LaserHandler::onButtonReleaseEvent(const PositionInputData& pos) {
  // We're killed after that, do I need to call a redraw without draw?
  std::cout << "buttonRelease" << std::endl;
  this->enabled = false;
  repaint ();

  // Was that a tap? Taken from (Base)StrokeHandler.  Not sure why I'm
  // responsible of checking that, this should be factorized.
  Control* control = xournal->getControl();
  Settings* settings = control->getSettings();

  if (settings->getStrokeFilterEnabled())
  {
    std::cout << "1" << std::endl;
    int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
    double strokeFilterIgnoreLength = NAN;

    settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
    double dpmm = settings->getDisplayDpi() / 25.4;
    double zoom = xournal->getZoom();
    double lengthSqrd = (pow(((pos.x - this->buttonDownPoint.x) / zoom), 2) +
                         pow(((pos.y - this->buttonDownPoint.y) / zoom), 2)) *
      pow(xournal->getZoom(), 2);
        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
        pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
      std::cout << "2" << std::endl;

      if (pos.timestamp - LaserHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
            std::cout << "3" << std::endl;

        this->userTapped = true;
        LaserHandler::lastStrokeTime = pos.timestamp;
        return;
      }
    }
    LaserHandler::lastStrokeTime = pos.timestamp;
  }

}

Rectangle<double> LaserHandler::getDrawBounds() {
  // Overapproximate the draw bounds to avoid floating point inaccuracies.
  const double cb = LASER_RADIUS + 1.0;
  // Get the bounds relative the the containing page.
  return {this->x - cb + redrawable->getX(),
          this->y - cb + redrawable->getY(),
          cb * 2, cb * 2};

}

void LaserHandler::repaint () {
  const Rectangle<double>&& bounds = this->getDrawBounds();

  // Use the main widget rather than the page rerenderRect, it's much faster.
  gtk_widget_queue_draw_area (xournal->getWidget(),
                              bounds.x, bounds.y, bounds.width, bounds.height);
}

void LaserHandler::onButtonPressEvent(const PositionInputData& pos) {
  std::cout << "buttonPress" << std::endl;
  this->enabled = true;
  this->buttonDownPoint.x = pos.x;
  this->buttonDownPoint.y = pos.y;
  this->startStrokeTime = pos.timestamp;
  this->x = pos.x;
  this->y = pos.y;
  repaint ();
}

void LaserHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
  // nothing to do
}

void LaserHandler::resetShapeRecognizer() { }
