###############################################################################
#
#  $Id: FilterWidget.py 565 2010-11-11 08:22:22Z weegreenblobbie $
#
#  Nsound is a C++ library and Python module for audio synthesis featuring
#  dynamic digital filters. Nsound lets you easily shape waveforms and write
#  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
#  use.
#
#  Copyright (c) 2010 to Present Nick Hilton
#
#  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
#
###############################################################################

###############################################################################
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
###############################################################################

###############################################################################
# Imports

import sys

import pygtk
pygtk.require("2.0")
import gtk

###############################################################################
# Enumerated types for Filters
class IRType:

    IIR = 0
    FIR = 1

    END = 2 # keep at end

    TYPES = [IIR, FIR]

    STRINGS = \
    [
        "IIR",
        "FIR",
    ]

class FilterType:

    LOW_PASS    = 0
    HIGH_PASS   = 1
    BAND_PASS   = 2
    BAND_REJECT = 3

    END         = 4 # keep at end

    TYPES = [LOW_PASS, HIGH_PASS, BAND_PASS, BAND_REJECT]

    STRINGS = \
    [
        "Low Pass",
        "High Pass",
        "Band Pass",
        "Band Reject",
    ]

###############################################################################
class FilterWidget(gtk.Frame):
    """A special frame that encapsulates the Nsound Filters."""

    ###########################################################################
    def __init__(
        self,
        ir_type,
        filter_type,
        filter_id,
        label_str):

        gtk.Frame.__init__(self, "<b>" + label_str + "</b>")

        if ir_type not in IRType.TYPES:
            raise ValueError("ir_type (%d) not valid" % ir_type)

        if filter_type not in FilterType.TYPES:
            raise ValueError("filter_type (%d) not valid" % filter_type)

        if filter_id < 0:
            raise ValueError("filter_id is < 0")

        self._id = filter_id

        self._changed_callback_func = self._noop_callback

        self._ir_type     = ir_type
        self._filter_type = filter_type
        self._is_enabled  = False
        self._order       = 4
        self._fc_low      = 440.0
        self._fc_high     = 1660.0
        self._ripple      = 1.0

        # GUI stuff

        # Tell the label to render the <b>bold</b> html tags
        label = self.get_label_widget()
        label.set_use_markup(True)

        # Frame stettings
        self.set_label_align(0.0, 0.5)
        self.set_shadow_type(gtk.SHADOW_ETCHED_IN)

        # Alignment & padding
        self._alignment = gtk.Alignment(
            xalign=0.5,
            yalign=0.5,
            xscale=1.0,
            yscale=1.0)

        self._alignment.set_padding(0,10,10,0)

        # Root horizontal box container
        self._hbox = gtk.HBox(homogeneous = False, spacing = 0)

        self._hbox.set_size_request(610, 40)

        self._alignment.add(self._hbox)
        self.add(self._alignment)

        # Create a set of vboxes to hold all the oscillator's settings
        self._vbox_ir_type     = None
        self._vbox_filter_type = None
        self._vbox_enabled     = None
        self._vbox_order       = None
        self._vbox_fc_low      = None
        self._vbox_fc_high     = None
        self._vbox_ripple      = None

        self._buttons = {}

        # Based on the widget type, stuff the hbox
        self._build_widget()

    ###########################################################################
    def _build_widget(self):

        #######################################################################
        # IR Type

        if self._vbox_ir_type == None:

            self._vbox_ir_type = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("IR Type")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_ir_type.add(label)

            button = gtk.combo_box_new_text()
            button.connect("changed", self._ir_type_callback)

            for i in range(IRType.END):
                button.insert_text(i, IRType.STRINGS[i])

            button.set_active(self._ir_type)

            self._vbox_ir_type.add(button)

            self._buttons["ir_type"] = button

        self._hbox.pack_start(self._vbox_ir_type, expand = False, fill = False)

        #######################################################################
        # Filter Type

        if self._vbox_filter_type == None:

            self._vbox_filter_type = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Filter Type")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_filter_type.add(label)

            button = gtk.combo_box_new_text()
            button.connect("changed", self._filter_type_callback)

            for i in range(FilterType.END):
                button.insert_text(i, FilterType.STRINGS[i])

            button.set_active(self._filter_type)

            self._vbox_filter_type.add(button)

            self._buttons["filter_type"] = button

        self._hbox.pack_start(self._vbox_filter_type, expand = False, fill = False)

        #######################################################################
        # Enabled

        if self._vbox_enabled == None:

            self._vbox_enabled = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Enabled")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0
            label.set_width_chars(8)

            self._vbox_enabled.add(label)

            if self._is_enabled:
                button = gtk.ToggleButton("ON")
                button.set_active(True)
            else:
                button = gtk.ToggleButton("OFF")
                button.set_active(False)

            button.connect("toggled", self._enabled_callback)

            self._vbox_enabled.add(button)

            self._buttons["enabled"] = button

        self._hbox.pack_start(self._vbox_enabled, expand = False, fill = False)

        #######################################################################
        # Order

        if self._vbox_order == None:

            self._vbox_order = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Order")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_order.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._order,
                    lower = 2,
                    upper = 4096,
                    step_incr = 2,
                    page_incr = 4,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 0)

            button.set_width_chars(6)

            button.connect("value-changed", self._order_callback)

            self._vbox_order.add(button)

            self._buttons["order"] = button

        self._hbox.pack_start(self._vbox_order, expand = False, fill = False)

        #######################################################################
        # FC Low

        if self._vbox_fc_low == None:

            self._vbox_fc_low = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Fc Low (Hz)")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_fc_low.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._fc_low,
                    lower = 0.001,
                    upper = 20000.0,
                    step_incr = 5.0,
                    page_incr = 10.0,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 4)

            button.set_width_chars(10)

            button.connect("value-changed", self._fc_low_callback, None)

            self._vbox_fc_low.add(button)

            self._buttons["fc_low"] = button

        if self._filter_type != FilterType.HIGH_PASS:
            self._hbox.pack_start(self._vbox_fc_low, expand = False, fill = False)

        #######################################################################
        # FC high

        if self._vbox_fc_high == None:

            self._vbox_fc_high = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Fc High (Hz)")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_fc_high.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._fc_high,
                    lower = 0.001,
                    upper = 20000.0,
                    step_incr = 5.0,
                    page_incr = 10.0,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 4)

            button.set_width_chars(10)

            button.connect("value-changed", self._fc_high_callback, None)

            self._vbox_fc_high.add(button)

            self._buttons["fc_high"] = button

        if self._filter_type != FilterType.LOW_PASS :
            self._hbox.pack_start(self._vbox_fc_high, expand = False, fill = False)

        #######################################################################
        # Ripple

        if self._vbox_ripple == None:

            self._vbox_ripple = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Ripple %")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_ripple.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._ripple,
                    lower = 0.0,
                    upper = 1.0,
                    step_incr = 0.01,
                    page_incr = 0.10,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 3)

            button.set_width_chars(6)

            button.connect("value-changed", self._ripple_callback)

            self._vbox_ripple.add(button)

            self._buttons["ripple"] = button

        if self._ir_type == IRType.IIR:
            self._hbox.pack_start(self._vbox_ripple, expand = False, fill = False)

        self._hbox.show_all()

    ###########################################################################
    # No operation
    def _noop_callback(self, widget, data=None):
        pass

    ###########################################################################
    def _ir_type_callback(self, widget, data=None):
        self.ir_type = widget.get_active()

    ###########################################################################
    def _filter_type_callback(self, widget, data=None):
        self.filter_type = widget.get_active()

    ###########################################################################
    def _enabled_callback(self, widget, data=None):
        self.enabled = widget.get_active()

    ###########################################################################
    def _order_callback(self, widget, data=None):
        self.order = int(widget.get_value())

    ###########################################################################
    def _fc_low_callback(self, widget, data=None):
        self.fc_low = widget.get_value()

    ###########################################################################
    def _fc_high_callback(self, widget, data=None):
        self.fc_high = widget.get_value()

    ###########################################################################
    def _ripple_callback(self, widget, data=None):
        self.ripple = widget.get_value()

    ###########################################################################
    @property
    def changed_callback(self):
        return self._changed_callback_func

    ###########################################################################
    @changed_callback.setter
    def changed_callback(self, func):
        self._changed_callback_func = func

    ###########################################################################
    @property
    def ir_type(self):
        self._ir_type = self._buttons["ir_type"].get_active()
        return self._ir_type

    ###########################################################################
    @ir_type.setter
    def ir_type(self, t):

        if t in IRType.TYPES and self._ir_type != t:

            self._ir_type = t

            # Empty the hvbox and repopulate it.
            for child in self._hbox.get_children():
                self._hbox.remove(child)

            self._build_widget()

            self._changed_callback_func(self, "filter_remake")

    ###########################################################################
    @property
    def filter_type(self):
        self._filter_type = self._buttons["filter_type"].get_active()
        return self._filter_type

    ###########################################################################
    @filter_type.setter
    def filter_type(self, t):

        if t in FilterType.TYPES and self._filter_type != t:

            self._filter_type = t

            # Empty the hbox
            for child in self._hbox.get_children():
                self._hbox.remove(child)

            self._build_widget()

            self._changed_callback_func(self, "filter_remake")

    ###########################################################################
    @property
    def enabled(self):
        self._is_enabled = self._buttons["enabled"].get_active()
        return self._is_enabled

    ###########################################################################
    @enabled.setter
    def enabled(self, flag):

        if self._is_enabled != flag:
            self._is_enabled = flag

            self._buttons["enabled"].set_active(self._is_enabled)

            if self._is_enabled:
                self._buttons["enabled"].set_label("ON")
            else:
                self._buttons["enabled"].set_label("OFF")

            self._changed_callback_func(self)

    ###########################################################################
    @property
    def order(self):
        self._order = int(self._buttons["order"].get_value())
        return self._order

    ###########################################################################
    @order.setter
    def order(self, v):
        if v < 2:
            v = 2

        if v > 4096:
            v = 4096

        if self._ir_type == IRType.IIR and v > 22:
            v = 20

        self._order = int(v)
        self._buttons["order"].set_value(self._order)

        self._changed_callback_func(self, "filter_remake")

    ###########################################################################
    @property
    def fc_low(self):
        self._fc_low = self._buttons["fc_low"].get_value()
        return self._fc_low

    ###########################################################################
    @fc_low.setter
    def fc_low(self, fc):
        if fc < 0.0:
            fc = 0.0

        if fc > 20000.0:
            fc = 20000.0

        self._fc_low = fc
        self._buttons["fc_low"].set_value(self._fc_low)

        self._changed_callback_func(self)

    ###########################################################################
    @property
    def fc_high(self):
        self._fc_high = self._buttons["fc_high"].get_value()
        return self._fc_high

    ###########################################################################
    @fc_high.setter
    def fc_high(self, fc):
        if fc < 0.0:
            fc = 0.0

        if fc > 20000.0:
            fc = 20000.0

        self._fc_high = fc
        self._buttons["fc_high"].set_value(self._fc_high)

        self._changed_callback_func(self)

    ###########################################################################
    @property
    def ripple(self):
        self._ripple = self._buttons["ripple"].get_value()
        return self._ripple

    ###########################################################################
    @ripple.setter
    def ripple(self, r):
        if r < 0.0:
            r = 0.001

        self._ripple = r
        self._buttons["ripple"].set_value(self._ripple)

        self._changed_callback_func(self, "filter_remake")

