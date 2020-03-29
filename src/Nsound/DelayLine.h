//-----------------------------------------------------------------------------
//
//  $Id: DelayLine.h 908 2015-07-08 02:04:41Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2007 Nick Hilton
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
#ifndef _NSOUND_DELAY_LINE_H_
#define _NSOUND_DELAY_LINE_H_

#include <Nsound/Nsound.h>

#include <vector>

namespace Nsound
{


class AudioStream;
class Buffer;


class DelayLine
{

public:

    DelayLine(float64 sample_rate, float64 max_delay_in_seconds);

    void setRealtime(bool flag) {is_realtime_ = flag;}

    Buffer delay(const Buffer & x, const Buffer & delay_time);

    float64 delay(float64 x, float64 delay_time);

    float64 read();
    float64 read(float64 delay);

    void write(float64 x);

    void reset(); // flushes internal state

protected:

    float64 sample_rate_;
    float64 max_delay_time_;
    float64 delay_time_;

    std::vector<float64> buffer_;

    uint32 wr_idx_;

    bool is_realtime_;

private:

    DelayLine(const DelayLine & copy);
    DelayLine & operator=(const DelayLine & rhs);
};


} // namespace


#endif
