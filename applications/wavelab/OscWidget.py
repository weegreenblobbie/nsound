###############################################################################
#
#  $Id: OscWidget.py 575 2010-12-06 01:18:04Z weegreenblobbie $
#
#  Nsound is a C++ library and Python module for audio synthesis featuring
#  dynamic digital filters. Nsound lets you easily shape waveforms and write
#  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
#  use.
#
#  Copyright (c) 2010 to Present Nick Hilton
#
#  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
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
# Enumerated type for oscilators.
class OscType:

    SINE     = 0
    SAWTOOTH = 1
    SQUARE   = 2
    PULSE    = 3
    NOISE    = 4

    END      = 5 # keep at end

    TYPES = [SINE, SAWTOOTH, SQUARE, PULSE, NOISE]

    STRINGS = \
    [
        "Sine",
        "Sawtooth",
        "Square",
        "Pulse",
        "Noise",
    ]

###############################################################################
# Enumerated type for Pulse Width Units.
class PulseUnits:

    MSEC     = 0
    PERCENT  = 1

    END      = 2 # keep at end

    TYPES = [MSEC, PERCENT]

    STRINGS = \
    [
        "msec",
        "% lambda",
    ]

###############################################################################
class OscWidget(gtk.Frame):
    """A special frame that encapsulates the Nsound Generators."""

    ###########################################################################
    def __init__(
        self,
        osc_type,
        osc_id,
        label_str,
        show_sync_button):

        gtk.Frame.__init__(self, "<b>" + label_str + "</b>")

        if osc_type not in OscType.TYPES:
            raise ValueError("osc_type (%d) not valid" % osc_type)

        if osc_id < 0:
            raise ValueError("osc_id is < 0")

        self._id = osc_id

        self._changed_callback_func = self._noop_callback

        self._type = osc_type
        self._can_sync = show_sync_button

        self._volume = 0.0
        self._pitch = 0.0
        self._harmonics = 3
        self._pulse_width = 0.01
        self._mute_is_on = True
        self._sync_is_on = False

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

        self._hbox.set_size_request(580, 40)

        self._alignment.add(self._hbox)
        self.add(self._alignment)

        # Create a set of vboxes to hold all the oscillator's settings
        self._vbox_type        = None
        self._vbox_mute        = None
        self._vbox_volume      = None
        self._vbox_pitch       = None
        self._vbox_sync        = None
        self._vbox_harmonics   = None
        self._vbox_pulse_width = None
        self._vbox_pulse_units = None

        self._buttons = {}

        # Based on the widget type, stuff the hbox
        self._build_widget()

    ###########################################################################
    def _build_widget(self):

        #######################################################################
        # Type

        if self._vbox_type == None:

            self._vbox_type = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Type")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_type.add(label)

            button = gtk.combo_box_new_text()
            button.connect("changed", self._type_callback)

            for i in range(OscType.END):
                button.insert_text(i, OscType.STRINGS[i])

            button.set_active(self._type)

            self._vbox_type.add(button)

            self._buttons["type"] = button

        self._hbox.pack_start(self._vbox_type, expand = False, fill = False)

        #######################################################################
        # Mute

        if self._vbox_mute == None:

            self._vbox_mute = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Mute")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0
            label.set_width_chars(6)

            self._vbox_mute.add(label)

            if self._mute_is_on:
                button = gtk.ToggleButton("ON")
                button.set_active(True)
            else:
                button = gtk.ToggleButton("OFF")
                button.set_active(False)

            button.connect("toggled", self._mute_callback)

            self._vbox_mute.add(button)

            self._buttons["mute"] = button

        self._hbox.pack_start(self._vbox_mute, expand = False, fill = False)

        #######################################################################
        # Volume

        if self._vbox_volume == None:

            self._vbox_volume = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Volume")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_volume.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._volume,
                    lower = 0.0,
                    upper = 1.0,
                    step_incr = 0.01,
                    page_incr = 0.10,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 2)

            button.set_width_chars(6)

            button.connect("value-changed", self._volume_callback)

            self._vbox_volume.add(button)

            self._buttons["volume"] = button

        self._hbox.pack_start(self._vbox_volume, expand = False, fill = False)

        #######################################################################
        # Pitch

        if self._vbox_pitch == None:

            self._vbox_pitch = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Pitch (Hz)")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0

            self._vbox_pitch.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._pitch,
                    lower = 0.001,
                    upper = 20000.0,
                    step_incr = 5.0,
                    page_incr = 10.0,
                    page_size = 0.0),
                climb_rate = 0.0,
                digits = 4)

            button.set_width_chars(10)

            button.connect("value-changed", self._pitch_callback, None)

            self._vbox_pitch.add(button)

            self._buttons["pitch"] = button

        if self._type != OscType.NOISE:
            self._hbox.pack_start(self._vbox_pitch, expand = False, fill = False)

        #######################################################################
        # Sync

        if self._vbox_sync == None:

            self._vbox_sync = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Sync")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 0
            label.ypad   = 0
            label.set_width_chars(6)

            self._vbox_sync.add(label)

            button = gtk.ToggleButton("OFF")

            button.connect("toggled", self._sync_callback)

            self._vbox_sync.add(button)

            self._buttons["sync"] = button

        if self._can_sync and self._type != OscType.NOISE:
            self._hbox.pack_start(self._vbox_sync, expand = False, fill = False)

        #######################################################################
        # Harmonics

        if self._vbox_harmonics == None:

            self._vbox_harmonics = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Harmonics")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 10
            label.ypad   = 0
            label.set_max_width_chars(10)

            self._vbox_harmonics.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = 3,
                    lower = 2,
                    upper = 256,
                    step_incr = 1,
                    page_incr = 5,
                    page_size = 0),
                climb_rate = 0,
                digits = 0)

            button.connect("value-changed", self._harmonics_callback)

            self._vbox_harmonics.add(button)

            self._buttons["harmonics"] = button

        if self._type in [OscType.SAWTOOTH, OscType.SQUARE]:
            self._hbox.pack_start(self._vbox_harmonics, expand = False, fill = False)

        #######################################################################
        # Pulse Width

        if self._vbox_pulse_width == None:

            self._vbox_pulse_width = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Pulse Width")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 10
            label.ypad   = 0
            label.set_max_width_chars(13)

            self._vbox_pulse_width.add(label)

            button = gtk.SpinButton(
                gtk.Adjustment(
                    value = self._pulse_width,
                    lower = 0.000001,
                    upper = 200000.0,
                    step_incr = 0.0005,
                    page_incr = 0.0010,
                    page_size = 0),
                climb_rate = 0,
                digits = 8)

            button.connect("value-changed", self._pulse_width_callback)

            self._vbox_pulse_width.add(button)

            self._buttons["pulse_width"] = button

        if self._type == OscType.PULSE:
            self._hbox.pack_start(self._vbox_pulse_width, expand = False, fill = False)

        #######################################################################
        # Pulse Units

        if self._vbox_pulse_units == None:

            self._vbox_pulse_units = gtk.VBox(homogeneous = False, spacing = 0)

            label = gtk.Label("Pulse Units")

            label.justify = gtk.JUSTIFY_LEFT
            label.xalign = 0.5
            label.yalign = 0.5
            label.xpad   = 10
            label.ypad   = 0
            label.set_max_width_chars(10)

            self._vbox_pulse_units.add(label)

            button = gtk.combo_box_new_text()
            button.insert_text(0, "msec")
            button.insert_text(0, "% lambda")
            button.set_active(0)

            button.connect("changed", self._pulse_units_callback)

            self._vbox_pulse_units.add(button)

            self._buttons["pulse_units"] = button

        if self._type == OscType.PULSE:
            self._hbox.pack_start(self._vbox_pulse_units, expand = False, fill = False)

        self._hbox.show_all()

    ###########################################################################
    def _mute_callback(self, widget, data=None):
        self.mute = widget.get_active()

    ###########################################################################
    def _volume_callback(self, widget, data=None):

        v = widget.get_value()
        self._volume = v
        self._changed_callback_func(self)

    ###########################################################################
    def _pitch_callback(self, widget, data=None):

        p = widget.get_value()
        self._pitch = p
        self._changed_callback_func(self)

    ###########################################################################
    def _sync_callback(self, widget, data=None):
        self.sync = widget.get_active()

    ###########################################################################
    def _harmonics_callback(self, widget, data=None):

        h = int(widget.get_value())
        self._harmonics = h
        self._changed_callback_func(self, "osc_remake")

    ###########################################################################
    def _pulse_width_callback(self, widget, data=None):

        pw = widget.get_value()
        self._pulse_width = pw
        self._changed_callback_func(self, "osc_remake")

    ###########################################################################
    def _pulse_units_callback(self, widget, data=None):

        pu = widget.get_active()
        self._pulse_units = pu
        self._changed_callback_func(self, "osc_pulse_units")

    ###########################################################################
    def _type_callback(self, widget, data=None):
        t = widget.get_active()
        self.type = t

    ###########################################################################
    def _noop_callback(self, widget, data=None):
        pass

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
    def type(self):
        self._type = self._buttons["type"].get_active()
        return self._type

    ###########################################################################
    @type.setter
    def type(self, t):

        if t in OscType.TYPES and self._type != t:

            self._type = t

            # Empty the hbox
            for child in self._hbox.get_children():
                self._hbox.remove(child)

            self._build_widget()

            self._changed_callback_func(self, "osc_remake")

    ###########################################################################
    @property
    def mute(self):
        self._mute_is_on = self._buttons["mute"].get_active()
        return self._mute_is_on

    ###########################################################################
    @mute.setter
    def mute(self, flag):

        if self._mute_is_on != flag:
            self._mute_is_on = flag

            self._buttons["mute"].set_active(self._mute_is_on)

            if self._mute_is_on:
                self._buttons["mute"].set_label("ON")
            else:
                self._buttons["mute"].set_label("OFF")

            self._changed_callback_func(self)

    ###########################################################################
    @property
    def volume(self):
        self._volume = self._buttons["volume"].get_value()
        return self._volume

    ###########################################################################
    @volume.setter
    def volume(self, v):
        if v < 0.0:
            v = 0.0

        if v > 1.0:
            v = 1.0

        self._volume = v
        self._buttons["volume"].set_value(self._volume)

        self._changed_callback_func(self)

    ###########################################################################
    @property
    def harmonics(self):
        self._harmonics = int(self._buttons["harmonics"].get_value())
        return self._harmonics

    ###########################################################################
    @harmonics.setter
    def harmonics(self, h):
        if h < 0:
            h = 1

        if h > 256:
            h = 256

        self._harmonics = int(h)
        self._buttons["harmonics"].set_value(self._harmonics)

        self._changed_callback_func(self, "osc_remake")

    ###########################################################################
    @property
    def pitch(self):
        self._pitch = self._buttons["pitch"].get_value()
        return self._pitch

    ###########################################################################
    @pitch.setter
    def pitch(self, p):
        if p < 0.0:
            p = 0.001

        self._pitch = p
        self._buttons["pitch"].set_value(self._pitch)

        self._changed_callback_func(self)

    ###########################################################################
    @property
    def sync(self):
        self._sync_is_on = self._buttons["sync"].get_active()
        return self._sync_is_on

    ###########################################################################
    @sync.setter
    def sync(self, flag):

        if self._sync_is_on != flag:
            self._sync_is_on = flag

            self._buttons["sync"].set_active(self._sync_is_on)

            if self._sync_is_on:
                self._buttons["sync"].set_label("ON")
            else:
                self._buttons["sync"].set_label("OFF")

            self._changed_callback_func(self, "osc_sync")

    ###########################################################################
    @property
    def pulse_width(self):
        self._pulse_width = self._buttons["pulse_width"].get_value()
        return self._pulse_width

    ###########################################################################
    @pulse_width.setter
    def pulse_width(self, pw):
        if pw < 0.0:
            pw = 0.0001

        self._pulse_width = pw
        self._buttons["pulse_width"].set_value(self._pulse_width)

        self._changed_callback_func(self)

    ###########################################################################
    @property
    def pulse_units(self):
        pu = self._buttons["pulse_units"].get_active()

        if pu in PulseUnits.TYPES:
            self._pulse_units = PulseUnits.MSEC

        return self._pulse_units

    ###########################################################################
    @sync.setter
    def pulse_units(self, pu):
        if pu in PulseUnits.TYPES:
            self._pulse_units = pu
            self._buttons["pulse_units"].set_active(self._pulse_units)

            self._changed_callback_func(self, "pulse_units")

