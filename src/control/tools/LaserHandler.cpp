#include "LaserHandler.h"

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

LaserHandler::LaserHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
  InputHandler(xournal, redrawable, page) {}

LaserHandler::~LaserHandler() {}

void LaserHandler::draw(cairo_t* cr) {
  if (!this->enabled)
    return;
  int dpiScaleFactor = xournal->getDpiScaleFactor();
  cairo_save(cr);
  cairo_scale (cr, dpiScaleFactor, dpiScaleFactor);
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
  repaint ();
  this->x = pos.x;
  this->y = pos.y;
  repaint ();
  return true;
}

void LaserHandler::onButtonReleaseEvent(const PositionInputData& pos) {
  // We're killed after that, do I need to call a redraw without draw?
  this->enabled = false;
  repaint ();
}

Rectangle<double> LaserHandler::getDrawBounds() {
  // Overapproximate the draw bounds to avoid floating point inaccuracies.
  const double cb = (LASER_RADIUS + 1.0);
  // Get the bounds relative the the containing page.  This is scaled afterwise
  // using dpiScaleFactor, so shouldn't do it here.
  return {this->x  - cb + redrawable->getX(),
          this->y  - cb + redrawable->getY(),
          cb * 2, cb * 2};

}

void LaserHandler::repaint () {
  const Rectangle<double>&& bounds = this->getDrawBounds();
  // Use the main widget rather than the page rerenderRect, it's much faster.
  gtk_widget_queue_draw_area (xournal->getWidget(),
                              bounds.x, bounds.y, bounds.width, bounds.height);
}

void LaserHandler::onButtonPressEvent(const PositionInputData& pos) {
  this->enabled = true;
  this->x = pos.x;
  this->y = pos.y;
  repaint ();
}

void LaserHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
  // nothing to do
}

void LaserHandler::resetShapeRecognizer() { }
