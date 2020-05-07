/*
 * Xournal++
 *
 * Handles laser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "view/DocumentView.h"

#include "InputHandler.h"

/**
 * @brief The laser handler draws a laser on a XojPageView
 *
 */
class LaserHandler: public InputHandler {
public:
    LaserHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    virtual ~LaserHandler();

    void draw(cairo_t* cr);

    bool onMotionNotifyEvent(const PositionInputData& pos);
    void onButtonReleaseEvent(const PositionInputData& pos);
    void onButtonPressEvent(const PositionInputData& pos);
    void onButtonDoublePressEvent(const PositionInputData& pos);
    bool onKeyEvent(GdkEventKey* event);
    /**
     * Reset the shape recognizer, only implemented by drawing instances,
     * but needs to be in the base interface.
     */
    virtual void resetShapeRecognizer();

  private:
    bool enabled = false;
    /** The current x coordinate in canvas coordinates */
    double x = 0.0;
    /** The current y coordinate in canvas coordinates */
    double y = 0.0;
    unsigned int color = 0xFF0000FF;
    static constexpr double LASER_RADIUS = 5.0;

    Rectangle<double> getDrawBounds();
    void repaint ();

    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    guint32 startStrokeTime{}; // Tap detection
    static guint32 lastStrokeTime; // Taken from StrokeHandler to detect taps.
};
