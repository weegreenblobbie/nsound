//-----------------------------------------------------------------------------
//
//  $Id: AudioPlaybackRt.i 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2009 to Present Nick Hilton
//
//  weegreenblobbie2_gmail_com (replace '_' with '@' and '.')
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//-----------------------------------------------------------------------------


%feature("shadow") Nsound::AudioPlaybackRt::_swig_shadow()
%{

#------------------------------------------------------------------------------
# from swig/shadowed.i
#
def __lshift__(self, rhs):
    self.play(rhs)

def __rshift__(self, rhs):
    self.play(rhs)

def __str__(self):
    return "Nsound.AudioPlaybackRt(): %s\n%s" % (self.getInfo(), self.debug_print())

__repr__ = __str__

#
#------------------------------------------------------------------------------

%}

// :mode=python:
