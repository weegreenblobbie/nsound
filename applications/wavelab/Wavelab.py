#!/usr/bin/env python

###############################################################################
#
#  $Id: Wavelab.py 658 2011-06-11 22:51:18Z weegreenblobbie $
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

# Globals

LINEAR = 0
LOG    = 1

ON = 0
OFF = 1

###############################################################################
# Imports

import os
import sys
import threading

try:
    import pygtk
    pygtk.require("2.0")
except:
    print >>sys.stderr, "Could not import pygtk 2.0 or better."
    sys.exit(1)

try:
    import gtk
except:
    print >>sys.stderr, "Could not import gtk."
    sys.exit(1)

try:
    import gtk.glade
except:
    print >>sys.stderr, "Could not import gtk.glade."
    sys.exit(1)

try:
    import gobject
except:
    print >>sys.stderr, "Could not import gobject."
    sys.exit(1)


try:
    import matplotlib
except:
    print >>sys.stderr, "Could not import matplotlib."
    sys.exit(1)

matplotlib.use('GTK')
from matplotlib.figure import Figure
#~from matplotlib.axes import Subplot

# AGG backend
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar

# Non-AGG
#~from matplotlib.backends.backend_gtk import FigureCanvasGTK as FigureCanvas
#~from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

from matplotlib import pylab

try:
    import Nsound
except:
    print >>sys.stderr, "Could not import Nsound."
    sys.exit(1)

from FilterWidget import *
from OscWidget import *

###############################################################################
class Wavelab:
    """This is the Wavelab application"""

    ###########################################################################
    def __init__(self):

        self._sample_rate = 44100.0
        self._duration = 1.0
        self._fft_size = 8192

        self._need_redraw = True
        self._need_redraw_lock = threading.Lock()

        #Set the Glade file
        self._gladefile = "layout2.glade"
        self._xml = gtk.glade.XML(self._gladefile)

        #Get the Main Window, and connect the "destroy" event
        self._window = self._xml.get_widget("main_window")

        self._window.set_title("Wavelab ~ Nsound-%d.%d.%d ~ %s" %(
            Nsound.NSOUND_VERSION_A,
            Nsound.NSOUND_VERSION_B,
            Nsound.NSOUND_VERSION_C,
            "http://nsound.sourceforge.net"))

        if self._window:
            self._window.connect("destroy", gtk.main_quit)

        menuitem = self._xml.get_widget("menu_file_quit")
        menuitem.connect("activate", gtk.main_quit)

        # Disable some of the menus for now
        menu = self._xml.get_widget("menu_file_new")
        menu.set_sensitive(False)

        menu = self._xml.get_widget("menu_file_save")
        menu.set_sensitive(False)

        menu = self._xml.get_widget("menu_file_saveas")
        menu.connect("activate", self.menu_save_as)

        # Disable the help for now.
        menu = self._xml.get_widget("menu_help")
        menu.set_sensitive(False)

        # Set text on menu_file_open
        menu = self._xml.get_widget("menu_file_open")
        menu.set_label("Open Reference")
        menu.connect("activate", self.open_reference)

        self._last_path = None
        self._reference_wave = None

        # Don't know how to get the default background color, so I'm hard
        # coding it here for now.

        fig_bg = (230.0/255.0, 221.0/255.0, 213.0/255.0)

        # Freq plot
        self.freq_fig = Figure(
            figsize = (12,8),
            dpi = 72,
            facecolor = fig_bg,
            edgecolor = fig_bg)

        self.freq_axis = self.freq_fig.add_subplot(111)

        self.freq_canvas = FigureCanvas(self.freq_fig) # a gtk.DrawingArea
#~        self.freq_canvas.show()

        vbox = self._xml.get_widget("vbox_freq")
        vbox.pack_start(self.freq_canvas, True, True)

        navbar = NavigationToolbar(self.freq_canvas, self._window)
        vbox.pack_start(navbar, False, False)

        # Time plot
        self.time_fig = Figure(
            figsize = (12,8),
            dpi = 72,
            facecolor = fig_bg,
            edgecolor = fig_bg)

        self.time_axis = self.time_fig.add_subplot(111)

        self.time_canvas = FigureCanvas(self.time_fig) # a gtk.DrawingArea
#~        self.time_canvas.show()

        vbox = self._xml.get_widget("vbox_time")
        vbox.pack_start(self.time_canvas, True, True)

        navbar = NavigationToolbar(self.time_canvas, self._window)
        vbox.pack_start(navbar, False, False)

        # Adjust bounding boxes
        self.freq_fig.subplots_adjust(
            left   = 0.10,
            right  = 0.95,
            top    = 0.95,
            bottom = 0.18)

        self.time_fig.subplots_adjust(
            left   = 0.10,
            right  = 0.95,
            top    = 0.95,
            bottom = 0.18)

        #######################################################################
        # Get the oscillator vbox
        osc_vbox = self._xml.get_widget("vbox_oscillators")

        self._osc_widgets = []

        # OSC1
        osc = OscWidget(OscType.SINE, 0, "OSC1", False)
        osc.mute = False
        osc.volume = 1.0
        osc.pitch = 440.0
        osc.changed_callback = self.callback_widget_changed

        self._osc_widgets.append(osc)

        osc_vbox.add(osc)

        # OSC2
        osc = OscWidget(OscType.SAWTOOTH, 1, "OSC2", True)
        osc.mute = True
        osc.volume = 1.0
        osc.pitch = 440.0
        osc.harmonics = 3
        osc.sync = False
        osc.changed_callback = self.callback_widget_changed

        self._osc_widgets.append(osc)

        osc_vbox.add(osc)

        # OSC3
        osc = OscWidget(OscType.SQUARE, 2, "OSC3", True)
        osc.mute = True
        osc.volume = 1.0
        osc.pitch = 440.0
        osc.harmonics = 3
        osc.changed_callback = self.callback_widget_changed
        osc.sync = False
        self._osc_widgets.append(osc)

        osc_vbox.add(osc)

        # OSC4
        osc = OscWidget(OscType.NOISE, 3, "OSC4", True)
        osc.mute = True
        osc.volume = 1.0
        osc.changed_callback = self.callback_widget_changed
        self._osc_widgets.append(osc)

        osc_vbox.add(osc)

        # Create default oscillators
        self._oscillators = [None, None, None, None]

        for widget in self._osc_widgets:
            self.remake_oscillator(widget)

        #######################################################################
        # Get the filter vbox
        filter_vbox = self._xml.get_widget("vbox_filters")

        self._filter_widgets = []

        # Filter1
        f = FilterWidget(IRType.IIR, FilterType.LOW_PASS, 0, "Filter1")
        f.enabled = False
        f.order = 2
        f.fc_low = 440.0
        f.ripple = 0.00
        f.changed_callback = self.callback_widget_changed

        self._filter_widgets.append(f)

        filter_vbox.add(f)

        # Filter2
        f = FilterWidget(IRType.IIR, FilterType.HIGH_PASS, 1, "Filter2")
        f.enabled = False
        f.order = 2
        f.fc_high = 5000.0
        f.ripple = 0.00
        f.changed_callback = self.callback_widget_changed

        self._filter_widgets.append(f)

        filter_vbox.add(f)

        # Filter3
        f = FilterWidget(IRType.IIR, FilterType.BAND_PASS, 2, "Filter3")
        f.enabled = False
        f.order = 6
        f.fc_low = 4000.0
        f.fc_high = 6000.0
        f.ripple = 0.00
        f.changed_callback = self.callback_widget_changed

        self._filter_widgets.append(f)

        filter_vbox.add(f)

        # Filter4
        f = FilterWidget(IRType.IIR, FilterType.BAND_REJECT, 3, "Filter4")
        f.enabled = False
        f.order = 10
        f.fc_low = 4000.0
        f.fc_high = 6000.0
        f.ripple = 0.00
        f.changed_callback = self.callback_widget_changed

        self._filter_widgets.append(f)

        filter_vbox.add(f)

        # Create default oscillators
        self._filters = [None, None, None, None]

        for widget in self._filter_widgets:
            self.remake_filter(widget)

        #######################################################################
        # hook up widgets

        self.freq_xscale_toggle = self._xml.get_widget("freq_xscale_toggle")

        self.fft_size_combobox = self._xml.get_widget("fft_size_combobox")

        #######################################################################
        # Setup defaults

        self.fft_size_combobox.set_active(2) # 8192

        #######################################################################
        # Hook up callbacks

        self.freq_xscale_toggle.connect("toggled", self.callback_freq_xyscale, None)

        self.fft_size_combobox.connect("changed", self.callback_fft_size, None)

        # FFTransform
        self.fft = Nsound.FFTransform(self._sample_rate)
        self.fft.setWindow(Nsound.HANNING)

        self._noise = None

        gobject.timeout_add(500, self.update_plots)

        self._window.show_all()

    ###########################################################################
    def open_reference(self, widget, filename = None):

        if filename == None:

            dialog =  gtk.FileChooserDialog(
                "Open Reference .wav file",
                action = gtk.FILE_CHOOSER_ACTION_OPEN,
                buttons = (gtk.STOCK_OPEN,
						   gtk.RESPONSE_OK,
                           gtk.STOCK_CANCEL,
                           gtk.RESPONSE_CANCEL))

            if self._last_path != None:
                dialog.set_current_folder(self._last_path)

            file_filter = gtk.FileFilter()
            file_filter.set_name("Wavefiles")
            file_filter.add_pattern("*.wav")
            dialog.add_filter(file_filter)

            response = dialog.run()

            if response != gtk.RESPONSE_OK:
                dialog.destroy()
                return

            filename = dialog.get_filename()
            dirname = os.path.dirname(filename)
            dialog.destroy()
            self._last_path = dirname

        # Load the wavefile
        try:
            a = Nsound.AudioStream(filename)
        except:
            raise RuntimeError("Could not read wavefile '%s'" % filename)

        a.normalize()

        ref_sr  = a.getSampleRate()
        self._duration = a.getDuration()

        # Set the global self._sample_rate
        if self._sample_rate != ref_sr:

            self._sample_rate = ref_sr

            for widget in self._osc_widgets:
                self.remake_oscillator(widget)

            for widget in self._filter_widgets:
                self.remake_filter(widget)

        # Save the reference Nsound.Buffer object
        self._need_redraw_lock.acquire()
        self._reference_wave = a[0]
        self._need_redraw = True
        self._need_redraw_lock.release()

    ###########################################################################
    def menu_save_as(self, widget, data=None):

        dialog =  gtk.FileChooserDialog(
            "Save .py file",
            action = gtk.FILE_CHOOSER_ACTION_SAVE,
            buttons = (gtk.STOCK_SAVE,
                       gtk.RESPONSE_OK,
                       gtk.STOCK_CANCEL,
                       gtk.RESPONSE_CANCEL))

        if self._last_path != None:
            dialog.set_current_folder(self._last_path)

        file_filter = gtk.FileFilter()
        file_filter.set_name("Python Files")
        file_filter.add_pattern("*py")
        dialog.add_filter(file_filter)

        response = dialog.run()

        if response != gtk.RESPONSE_OK:
            dialog.destroy()
            return

        filename = dialog.get_filename()
        dirname = os.path.dirname(filename)
        dialog.destroy()

        self._last_path = dirname

        fd = open(filename, "w")

        fd.write(
"""# Automatically generated with %s

import Nsound

###############################################################################
def play(duration):

    sample_rate = %f

    out = Nsound.AudioStream(sample_rate, 1)

    # Create the oscillators
""" %(self._window.get_title(), self._sample_rate))


        #######################################################################
        # Create the oscillators
        for widget in self._osc_widgets:

            if widget.type == OscType.SINE:

                fd.write("    osc%d = Nsound.Sine(sample_rate)\n" %(
                    widget._id))

            elif widget.type == OscType.SAWTOOTH:
                fd.write("    osc%d = Nsound.Sawtooth(sample_rate, %d)\n" %(
                    widget._id,
                    widget.harmonics))

            elif widget.type == OscType.SQUARE:
                fd.write("    osc%d = Nsound.Square(sample_rate, %d)\n" %(
                    widget._id,
                    widget.harmonics))

            elif widget.type == OscType.PULSE:
                fd.write("    osc%d = Nsound.Pulse(sample_rate, %f)\n" %(
                    widget._id,
                    widget.pulse_width))

            if widget.sync:
                fd.write("    osc0.addSlaveSync(osc%d)\n" % widget._id)

        fd.write("""
    out << osc0.drawLine(duration, 0.0, 0.0)
""")

        # Generate the sound
        for widget in self._osc_widgets:
            amp   = widget.volume
            pitch = widget.pitch

            if widget.mute:
                amp *= 0.0

            if widget.type != OscType.NOISE:

                fd.write("""
    amp   = %f
    pitch = %f

    out += amp * osc%d.generate(duration, pitch)
""" %(              amp,
                    pitch,
                    widget._id))

            else:

                fd.write("""
    amp = %f
    out += amp * osc0.whiteNoise(duration)

    # Create the filters
""" % amp)

        #######################################################################
        # create filters
        for widget in self._filter_widgets:

            if not widget.enabled:
                continue

            if widget.ir_type == IRType.IIR:

                if widget.filter_type == FilterType.LOW_PASS:

                    fd.write("""
    filter%d = Nsound.FilterLowPassIIR(
        sample_rate,
        %d,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low,
                        widget.ripple))

                elif widget.filter_type == FilterType.HIGH_PASS:

                    fd.write("""
    filter%d = Nsound.FilterHighPassIIR(
        sample_rate,
        %d,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_high,
                        widget.ripple))

                elif widget.filter_type == FilterType.BAND_PASS:

                    fd.write("""
    filter%d = Nsound.FilterBandPassIIR(
        sample_rate,
        %d,
        %f,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low,
                        widget.fc_high,
                        widget.ripple))

                elif widget.filter_type == FilterType.BAND_REJECT:

                    fd.write("""
    filter%d = Nsound.FilterBandRejectIIR(
        sample_rate,
        %d,
        %f,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low,
                        widget.fc_high,
                        widget.ripple))

            elif widget.ir_type == IRType.FIR:

                if widget.filter_type == FilterType.LOW_PASS:

                    fd.write("""
    filter%d = Nsound.FilterLowPassFIR(
        sample_rate,
        %d,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low))

                elif widget.filter_type == FilterType.HIGH_PASS:

                    fd.write("""
    filter%d = Nsound.FilterHighPassFIR(
        sample_rate,
        %d,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_high))

                elif widget.filter_type == FilterType.BAND_PASS:

                    fd.write("""
    filter%d = Nsound.FilterBandPassFIR(
        sample_rate,
        %d,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low,
                        widget.fc_high))

                elif widget.filter_type == FilterType.BAND_REJECT:

                    fd.write("""
    filter%d = Nsound.FilterBandRejectFIR(
        sample_rate,
        %d,
        %f,
        %f)
""" %(
                        widget._id,
                        widget.order,
                        widget.fc_low,
                        widget.fc_high))

        #######################################################################
        # Filter the sound
        fd.write("\n    # Filter the sound\n")
        for widget in self._filter_widgets:

            if widget.enabled:
                fd.write("    out = filter%d.filter(out)\n" % widget._id)


        fd.write("""
    out.normalize()

    return out

""")

        fd.close()


    ###########################################################################
    def remake_oscillator(self, widget):

        self._need_redraw_lock.acquire()

        if widget.type == OscType.SINE:
            self._oscillators[widget._id] = Nsound.Sine(self._sample_rate)

        elif widget.type == OscType.SAWTOOTH:
            self._oscillators[widget._id] = Nsound.Sawtooth(
                self._sample_rate,
                widget.harmonics)

        elif widget.type == OscType.SQUARE:
            self._oscillators[widget._id] = Nsound.Square(
                self._sample_rate,
                widget.harmonics)

        elif widget.type == OscType.PULSE:

            self._oscillators[widget._id] = Nsound.Pulse(
                self._sample_rate,
                widget.pulse_width)

        if widget.sync:
            self._oscillators[0].addSlaveSync(self._oscillators[widget._id])

        self._need_redraw = True
        self._need_redraw_lock.release()

    ###########################################################################
    def remake_filter(self, widget):

        self._need_redraw_lock.acquire()

        if widget.ir_type == IRType.IIR:

            if widget.filter_type == FilterType.LOW_PASS:
                f = Nsound.FilterLowPassIIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low,
                    widget.ripple)

            elif widget.filter_type == FilterType.HIGH_PASS:
                f = Nsound.FilterHighPassIIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_high,
                    widget.ripple)

            elif widget.filter_type == FilterType.BAND_PASS:
                f = Nsound.FilterBandPassIIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low,
                    widget.fc_high,
                    widget.ripple)

            elif widget.filter_type == FilterType.BAND_REJECT:
                f = Nsound.FilterBandRejectIIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low,
                    widget.fc_high,
                    widget.ripple)

        elif widget.ir_type == IRType.FIR:

            if widget.filter_type == FilterType.LOW_PASS:
                f = Nsound.FilterLowPassFIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low)

            elif widget.filter_type == FilterType.HIGH_PASS:
                f = Nsound.FilterHighPassFIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_high)

            elif widget.filter_type == FilterType.BAND_PASS:
                f = Nsound.FilterBandPassFIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low,
                    widget.fc_high)

            elif widget.filter_type == FilterType.BAND_REJECT:
                f = Nsound.FilterBandRejectFIR(
                    self._sample_rate,
                    widget.order,
                    widget.fc_low,
                    widget.fc_high)

        self._filters[widget._id] = f

        self._need_redraw = True
        self._need_redraw_lock.release()


    ###########################################################################
    def callback_fft_size(self, widget, data=None):

        self._need_redraw_lock.acquire()

        y = widget.get_active_text()

        self._fft_size = int(y)

        self._need_redraw = True
        self._need_redraw_lock.release()

    ###########################################################################
    def callback_widget_changed(self, widget, data=None):

        if data == "osc_remake":

            self.remake_oscillator(widget)

        elif data == "osc_sync":

            self._need_redraw_lock.acquire()

            if widget.sync:
                self._oscillators[0].addSlaveSync(
                    self._oscillators[widget._id])
            else:
                self._oscillators[0].removeSlaveSync(
                    self._oscillators[widget._id])

            self._need_redraw = True
            self._need_redraw_lock.release()

        elif data == "osc_pulse_units":
            pass

        elif data == "filter_remake":
            self.remake_filter(widget)

        else:
            self._need_redraw_lock.acquire()
            self._need_redraw = True
            self._need_redraw_lock.release()


    ###########################################################################
    def callback_freq_xyscale(self, widget, data=None):

        self._need_redraw_lock.acquire()

        if widget.get_active() == LINEAR:
            widget.set_label("Linear")
        else:
            widget.set_label("Log")

        self._need_redraw = True
        self._need_redraw_lock.release()

    ###########################################################################
    def update_plots(self):

        self._need_redraw_lock.acquire()

        if not self._need_redraw:
            self._need_redraw_lock.release()
            gobject.timeout_add(500, self.update_plots)
            return

        #######################################################################
        # Create random noise only once for this application
        if self._noise is None:
            self._noise = self._oscillators[0].whiteNoise(self._duration)

        #######################################################################
        # Create waveform
        wave = Nsound.AudioStream(self._duration, 1);

        # Fill with zeros
        wave << self._oscillators[0].drawLine(self._duration, 0.0, 0.0)

        freqs = []

        #######################################################################
        # Generate sound
        for widget in self._osc_widgets:

            amp = widget.volume
            freq = widget.pitch

            if widget.mute:
                amp = 0.0

            if widget.type != OscType.NOISE:
                freqs.append(freq)
                wave += amp * self._oscillators[widget._id].generate(
                    self._duration, freq)

            else:
                wave += amp * self._noise

        # Always add the first freq
        if len(freqs) == 0:
            freqs.append(self._osc_widgets[0].pitch)

        #######################################################################
        # Filter the sound
        for widget in self._filter_widgets:

            if widget.enabled:

                if widget.filter_type == FilterType.LOW_PASS:

                    wave = self._filters[widget._id].filter(wave, widget.fc_low)

                elif widget.filter_type == FilterType.HIGH_PASS:

                    wave = self._filters[widget._id].filter(wave, widget.fc_high)

                else:

                    fc_low = widget.fc_low
                    fc_high = widget.fc_high

                    wave = self._filters[widget._id].filter(wave, fc_low, fc_high)

        #######################################################################
        # Time Plot

        limits = self.time_axis.axis()

        self.time_axis.cla();
        self.time_axis.grid(True);

        # Plot waveform
        x_axis = self._oscillators[0].drawLine(self._duration, 0.0, self._duration)

        self.time_axis.plot(x_axis.toList(), wave[0].toList())

        if self._reference_wave is None:
            pass

        else:

            ref = self._reference_wave

            if x_axis.getLength() > ref.getLength():
                x_axis = x_axis.subbuffer(0,ref.getLength())

            elif x_axis.getLength() < ref.getLength():
                ref = ref.subbuffer(0,x_axis.getLength())

            self.time_axis.plot(
                x_axis.toList(),
                ref.toList(),
                color="red")

        self.time_axis.set_xlabel("Time (s)")
        self.time_axis.set_ylabel("Amplitude")

        freq_min = min(freqs)

        # Limits, must be last
        if  limits[0] == 0.0 and limits[1] == 1.0 and \
            limits[2] == 0.0 and limits[3] == 1.0:

            # Limit x axis to 6 cycles
            x_min = x_axis[0]
#~            x_max = 6.0 / freq_min
            x_max = self._duration

            x_dist = x_max - x_min

            x_min -= 0.02 * x_dist
            x_max += 0.02 * x_dist

            self.time_axis.axis(xmin = x_min, xmax = x_max)

            y_min = -2
            y_max = 2

            # Take min/max
            y_min = min([y_min, wave.getMin()])
            y_max = max([y_max, wave.getMax()])

            y_dist = y_max - y_min

            #~        y_min -= 0.05 * y_dist
            #~        y_max += 0.05 * y_dist

            y_min -= 1e-19
            y_max += 1e-19

            self.time_axis.axis(ymin = y_min, ymax = y_max)

        else:
            self.time_axis.axis(limits)

        #######################################################################
        # Frequency Plot

        limits = self.freq_axis.axis()

        self.freq_axis.cla();
        self.freq_axis.grid(True);

        vec = self.fft.fft(wave[0], self._fft_size)

        x = vec[0].getFrequencyAxis()
        y = vec[0].getMagnitude().getdB()

        self.freq_axis.plot(x,y)
        self.freq_axis.fill_between(x, y, -1000)

        if self._reference_wave is None:
            pass
        else:
            ref_vec = self.fft.fft(self._reference_wave, self._fft_size)
            y2 = ref_vec[0].getMagnitude().getdB()
            self.freq_axis.plot(x,y2.toList(), color="red")

        # Set axis scaling
        if self.freq_xscale_toggle.get_active() == LINEAR:
            self.freq_axis.set_xscale("linear")
        else:
            self.freq_axis.set_xscale("log")

        self.freq_axis.set_xlabel("Frequency (Hz)")
        self.freq_axis.set_ylabel("Magnitude (dB)")

        # Set limits, must be last
        if  limits[0] == 0.0 and limits[1] == 1.0 and \
            limits[2] == 0.0 and limits[3] == 1.0:

            y_min = -60.0
            y_max = y.getMax()
            y_dist = y_max - y_min

            y_max += 0.05 * y_dist

            self.freq_axis.axis(xmin = -500.0, ymin = y_min, ymax = y_max)

        else:
            self.freq_axis.axis(limits)

        # Refresh screen
        self.freq_canvas.draw()
        self.time_canvas.draw()

        self._need_redraw = False
        self._need_redraw_lock.release()

        gobject.timeout_add(500, self.update_plots)

if __name__ == "__main__":
    wl = Wavelab()
    gtk.main()

