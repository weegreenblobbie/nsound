//-----------------------------------------------------------------------------
//
//  $Id: OrganPipe.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009-Present Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/OrganPipe.h>
#include <Nsound/FilterLowPassIIR.h>
#include <Nsound/FilterTone.h>
#include <Nsound/Sine.h>
#include <Nsound/Sawtooth.h>
#include <Nsound/Square.h>

#include <Nsound/Wavefile.h> // for operator>>

// DEBUG
#include <Nsound/Plotter.h>


using namespace Nsound;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------
OrganPipe::
OrganPipe(const float64 & sample_rate)
    :
    Instrument(sample_rate),
    _sine(std::make_unique<Sine>(sample_rate))
{
}

AudioStream
OrganPipe::
play(
    const float64 & duration,
    const float64 & frequency)
{
    return play(duration, frequency, 0.5);
}


//-----------------------------------------------------------------------------
// Principla Stops for the Friesach, Pfarrkirche, St. Bartholomäus, Austria
// pipe organ, courtesy of Piotr Grabowski.
// See https://piotrgrabowski.pl/friesach/
//
// I used a genetic algorithm to fit synthesis parameters to match the spectrum
// for the steady state of each key, then construted this table.

namespace
{

struct Parameters
{
    float64              scale;
    std::vector<float64> phases;
    std::vector<float64> weights;
};

const std::map<float64, Parameters> principal_stops = {
// D1
{   29.56700, {1.25,
              {0.000000, 0.000000, 0.163347, 0.681676, 0.000000, 0.026011, 0.000000, 0.000000, },
              {1.000000, 0.581735, 0.644902, 0.206152, 0.234684, 0.248492, 0.151395, 0.047928, }}
},
// E1
{   35.51480, {1.15,
              {0.000000, 0.014087, 0.376986, 0.796405, 0.364711, 0.000000, 0.842400, 0.000000, 1.000000, 0.000000, 0.000000, 0.865091, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, },
              {0.819303, 0.241672, 1.000000, 0.234663, 0.201695, 0.149165, 0.029976, 0.000000, 0.032245, 0.000000, 0.000000, 0.023646, 0.000000, 0.032215, 0.000000, 0.020883, 0.017119, }}
},
// C1
{   35.70020, {1.15,
              {0.000000, 0.724599, 1.000000, 0.000000, 0.000000, 0.000000, 0.201206, 0.350472, 0.000000, },
              {1.000000, 0.849923, 0.054118, 0.200863, 0.000000, 0.000000, 0.031276, 0.031657, 0.017140, }}
},
// C1#
{   36.85730, {1.15,
              {0.000000, 0.392965, 0.000000, 0.000000, 0.000000, 0.000000, 0.814494, 0.000000, 0.000000, 0.084002, },
              {0.183031, 1.000000, 0.000000, 0.040020, 0.014831, 0.000000, 0.013244, 0.000000, 0.013341, 0.010963, }}
},
// D1#
{   42.75080, {0.75,
              {0.000000, 0.403226, 0.000000, 0.148062, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.517267, 0.000000, 0.000000, 0.000000, },
              {1.000000, 0.560227, 0.000000, 0.409493, 0.132809, 0.072734, 0.000000, 0.015413, 0.000000, 0.025174, 0.000000, 0.000000, 0.011130, }}
},
// F1
{   46.50290, {0.95,
              {0.000000, 0.118822, 0.000000, 1.000000, 0.018393, 0.000000, 0.000000, 0.102054, 0.396281, 1.000000, 0.000000, 0.000000, 0.000000, },
              {0.202085, 0.264224, 1.000000, 0.785820, 0.244130, 0.100779, 0.119090, 0.057147, 0.019017, 0.052863, 0.024058, 0.019456, 0.011599, }}
},
// F1#
{   48.75050, {0.95,
              {0.000000, 0.000000, 0.710746, 0.568415, 0.055642, 0.000000, 0.000000, 0.811713, 0.000000, 0.000000, 0.000000, },
              {0.436815, 1.000000, 0.091445, 0.200547, 0.038350, 0.000000, 0.016537, 0.019672, 0.000000, 0.000000, 0.011161, }}
},
// G1#
{   53.00570, {0.95,
              {0.000000, 0.697444, 0.000000, 0.910966, 1.000000, 0.000000, 0.000000, 0.000000, 0.338188, 0.000000, 0.000000, 0.000000, 0.666959, 0.000000, },
              {1.000000, 0.880841, 0.724171, 0.167549, 0.057940, 0.074174, 0.027510, 0.000000, 0.011983, 0.000000, 0.000000, 0.000000, 0.016271, 0.018162, }}
},
// G1
{   53.04160, {1.00,
              {0.000000, 0.758499, 0.459933, 0.725322, 0.321508, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.833761, },
              {1.000000, 0.332847, 0.168344, 0.222310, 0.074807, 0.097513, 0.034252, 0.000000, 0.000000, 0.000000, 0.000000, 0.010700, }}
},
// A1#
{   55.31260, {1.00,
              {0.000000, 0.000000, 0.241508, 1.000000, 0.149215, 0.302673, 0.000000, 0.000000, 0.000000, 0.000000, 0.151474, },
              {1.000000, 0.407420, 0.359594, 0.096032, 0.065941, 0.018671, 0.035932, 0.019239, 0.025386, 0.000000, 0.016056, }}
},
// A1
{   55.42510, {1.00,
              {0.000000, 0.000000, 0.915111, 0.350491, 0.000000, 0.000000, 0.000000, 0.000000, 0.881664, 0.000000, 0.000000, 0.224318, 0.000000, },
              {0.941892, 1.000000, 0.761651, 0.304627, 0.076888, 0.094258, 0.068654, 0.037628, 0.028191, 0.000000, 0.000000, 0.012779, 0.010676, }}
},
// B1
{   62.42330, {1.00,
              {0.000000, 1.000000, 1.000000, 0.000000, 0.411553, 0.000000, 1.000000, 0.756076, 0.239847, 0.563007, 0.000000, 0.000000, 0.758269, },
              {1.000000, 0.684112, 0.315656, 0.072992, 0.307321, 0.225607, 0.033593, 0.085979, 0.033488, 0.044206, 0.000000, 0.010781, 0.012087, }}
},
// C2
{   65.77770, {1.13,
              {0.000000, 0.752780, 0.442409, 0.879796, },
              {1.000000, 0.134496, 0.054940, 0.016071, }}
},
// C2#
{   69.68970, {1.30,
              {0.000000, 0.735910, 0.541613, 1.000000, 0.000000, 0.000000, 0.053366, 1.000000, },
              {1.000000, 0.302117, 0.030551, 0.030537, 0.024253, 0.000000, 0.011658, 0.017574, }}
},
// D2
{   73.71350, {1.35,
              {0.000000, 0.010070, 0.414973, 0.662821, 0.000000, 0.000000, 0.000000, 0.300457, 0.000000, },
              {1.000000, 0.266480, 0.163824, 0.024789, 0.010002, 0.010196, 0.000000, 0.012195, 0.012039, }}
},
// D2#
{   78.09970, {0.75,
              {0.000000, 0.609573, 0.181768, 1.000000, 0.000000, 0.000000, 0.909171, 0.521061, 1.000000, },
              {1.000000, 0.332774, 0.108176, 0.020410, 0.000000, 0.000000, 0.014483, 0.012718, 0.010609, }}
},
// E2
{   82.73730, {0.75,
              {0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.204210, },
              {0.923809, 1.000000, 0.124560, 0.045116, 0.028753, 0.018139, 0.030770, 0.052673, 0.000000, 0.012973, }}
},
// F2
{   87.49830, {0.75,
              {0.000000, 0.227767, 0.359049, 0.261285, 0.000000, 0.000000, 0.000000, },
              {1.000000, 0.777699, 0.219627, 0.044312, 0.011453, 0.000000, 0.016116, }}
},
// F2#
{   92.77190, {0.75,
              {0.000000, 1.000000, 1.000000, 0.701348, 0.459097, 0.000000, 1.000000, 0.133385, 1.000000, },
              {1.000000, 0.245536, 0.471524, 0.033510, 0.030336, 0.030015, 0.020381, 0.011877, 0.012779, }}
},
// G2
{   98.10700, {0.60,
              {0.000000, 0.924706, 0.525964, 1.000000, 0.025201, 0.526047, 0.000000, 0.155186, 0.873468, 0.000000, },
              {0.605384, 1.000000, 0.605690, 0.050033, 0.061895, 0.159174, 0.060057, 0.041207, 0.028088, 0.019093, }}
},
// G2#
{  103.92800, {0.60,
              {0.000000, 0.615306, 0.061026, 0.000000, 0.108142, 0.688900, },
              {1.000000, 0.122036, 0.144378, 0.011642, 0.013848, 0.011096, }}
},
// A2
{  110.27300, {0.90,
              {0.000000, 0.048887, 1.000000, 1.000000, 0.000000, 0.995516, 0.000000, 0.000000, 1.000000, },
              {1.000000, 0.124593, 0.237378, 0.029087, 0.036325, 0.019601, 0.015720, 0.000000, 0.011841, }}
},
// A2#
{  116.75600, {0.90,
              {0.000000, 0.312194, 0.000000, 1.000000, 0.708564, 0.769644, 0.380880, 0.000000, 0.811589, },
              {1.000000, 0.175930, 0.075392, 0.057911, 0.024938, 0.010914, 0.018178, 0.000000, 0.010650, }}
},
// B2
{  124.14600, {0.75,
              {0.000000, 1.000000, 0.762317, 0.000000, 0.000000, 0.000000, 0.090460, 0.013500, },
              {1.000000, 0.545576, 0.147392, 0.109298, 0.033392, 0.000000, 0.045194, 0.013586, }}
},
// C3
{  131.12400, {0.90,
              {0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.336698, 0.000000, },
              {1.000000, 0.532138, 0.145750, 0.029118, 0.032752, 0.019109, 0.024931, }}
},
// C3#
{  138.91500, {0.45,
              {0.000000, 1.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.074964, 0.613947, 0.703597, 0.450802, 0.013540, },
              {0.374645, 1.000000, 0.326857, 0.257265, 0.198019, 0.133056, 0.071164, 0.060482, 0.072952, 0.031896, 0.015097, 0.011796, 0.012808, }}
},
// D3
{  147.31400, {0.45,
              {0.000000, 1.000000, 0.640024, 0.512513, 0.194542, 1.000000, 0.819689, 0.358266, 0.776898, 0.000000, 0.000000, },
              {0.789180, 1.000000, 0.686098, 0.288650, 0.146367, 0.166516, 0.067316, 0.030986, 0.028834, 0.025410, 0.019166, }}
},
// D3#
{  156.34800, {0.40,
              {0.000000, 1.000000, 0.627153, 0.000000, 0.359881, 0.000000, 0.931946, 0.000000, 0.000000, 0.826616, },
              {1.000000, 0.139917, 0.155525, 0.102423, 0.043958, 0.052734, 0.016708, 0.036361, 0.000000, 0.016892, }}
},
// E3
{  165.33100, {0.40,
              {0.000000, 0.000000, 0.000000, 0.849763, 0.338018, 0.000000, 0.000000, 0.244882, 0.000000, 0.685534, },
              {1.000000, 0.156316, 0.219357, 0.097865, 0.027480, 0.023191, 0.022567, 0.012196, 0.000000, 0.012740, }}
},
// F3
{  175.39400, {0.35,
              {0.000000, 1.000000, 0.297174, 1.000000, 0.039392, 0.000000, 0.309015, 0.000000, 0.000000, 0.000000, 0.120221, },
              {1.000000, 0.577607, 0.379672, 0.110883, 0.033774, 0.038207, 0.055964, 0.033178, 0.000000, 0.000000, 0.010274, }}
},
// F3#
{  185.55500, {0.35,
              {0.000000, 0.000000, 0.000000, 1.000000, 0.857416, 0.224525, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.823380, 0.187002, },
              {1.000000, 0.261337, 0.264446, 0.030581, 0.110103, 0.036612, 0.000000, 0.012421, 0.019673, 0.013027, 0.000000, 0.014335, 0.010843, }}
},
// G3
{  196.62300, {0.35,
              {0.000000, 0.033810, 0.668947, 0.000000, 0.836954, 0.339790, 0.954437, 0.000000, 0.000000, 0.000000, 0.000000, },
              {0.800919, 1.000000, 0.244137, 0.012583, 0.083939, 0.054856, 0.064030, 0.026467, 0.015024, 0.014674, 0.010712, }}
},
// A3 ???
// B3 ???
// C4
{  262.63000, {0.45,
              {0.000000, 0.000000, 0.260832, 0.043430, 0.924308, 0.443267, 0.637123, 0.000000, 0.000000, },
              {0.908085, 0.302507, 1.000000, 0.433641, 0.178382, 0.115291, 0.017097, 0.000000, 0.040049, }}
},
// C4#
{  278.42000, {0.45,
              {0.000000, 0.000000, 0.000000, 0.305478, 0.013347, 0.000000, 0.000000, 0.400528, 1.000000, },
              {1.000000, 0.265296, 0.070350, 0.085498, 0.049296, 0.047249, 0.028273, 0.015718, 0.012946, }}
},
// D4
{  294.71600, {0.30,
              {0.000000, 0.000000, 0.621144, 0.938060, 0.000000, 0.000000, 0.000000, 0.040892, 0.000000, 0.857851, 0.128092, 0.399765, },
              {1.000000, 0.781286, 0.203586, 0.046525, 0.077761, 0.018120, 0.010194, 0.031181, 0.019905, 0.017829, 0.010792, 0.010119, }}
},
// D4#
{  312.45800, {0.30,
              {0.000000, 1.000000, 0.000000, 1.000000, 1.000000, 0.000000, 0.426548, 0.212235, 0.000000, 0.000000, 0.000000, 0.394383, },
              {1.000000, 0.326106, 0.022897, 0.141918, 0.011012, 0.038243, 0.021676, 0.020979, 0.012553, 0.010639, 0.000000, 0.010851, }}
},
// E4
{  330.96600, {0.50,
              {0.000000, 0.641204, 0.000000, 0.000000, 0.000000, 0.000000, 0.750665, 0.617529, },
              {0.239897, 1.000000, 0.177226, 0.124179, 0.033931, 0.011942, 0.025935, 0.010148, }}
},
// F4
{  350.50500, {0.40,
              {0.000000, 0.542722, 0.000000, 0.154881, 0.024146, 0.293994, 0.000000, 0.591195, 0.376315, 0.000000, 0.766986, 0.000000, },
              {1.000000, 0.475984, 0.365607, 0.102212, 0.047433, 0.057906, 0.030354, 0.014389, 0.012424, 0.000000, 0.014134, 0.011425, }}
},
// F4#
{  371.16600, {0.30,
              {0.000000, 0.284476, 0.095864, 0.000000, 0.847245, 0.869526, 0.000000, 0.000000, 0.078792, 0.469990, },
              {1.000000, 0.269044, 0.251806, 0.017892, 0.124789, 0.020254, 0.033194, 0.000000, 0.011874, 0.012336, }}
},
// G4
{  393.27200, {0.25,
              {0.000000, 1.000000, 0.404407, 1.000000, 0.000000, 1.000000, 1.000000, 0.674254, 0.000000, 1.000000, 0.378311, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, },
              {1.000000, 0.467368, 0.039249, 0.117235, 0.173539, 0.110134, 0.036499, 0.029239, 0.023988, 0.037179, 0.016912, 0.013396, 0.000000, 0.000000, 0.000000, 0.011957, }}
},
// G4#
{  416.67300, {0.25,
              {0.000000, 0.174803, 1.000000, 0.000000, 1.000000, 0.723516, 0.000000, 0.982333, 0.059128, 0.000000, 0.000000, },
              {0.491209, 1.000000, 0.342385, 0.099628, 0.140040, 0.087804, 0.047428, 0.019674, 0.014141, 0.015282, 0.011635, }}
},
// A4
{  441.66000, {0.20,
              {0.000000, 1.000000, 0.040482, 0.659146, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.643882, },
              {1.000000, 0.769964, 0.225149, 0.495462, 0.113568, 0.055514, 0.030237, 0.013756, 0.016442, 0.013314, }}
},
// A4#
{  468.20600, {0.20,
              {0.000000, 0.068456, 1.000000, 0.000000, 0.000000, 1.000000, 0.031125, 0.000000, 0.182576, 1.000000, },
              {1.000000, 0.194542, 0.127216, 0.149330, 0.000000, 0.018519, 0.019671, 0.000000, 0.012665, 0.011935, }}
},
// B4
{  495.07200, {0.30,
              {0.000000, 1.000000, 0.475652, 0.107220, 0.000000, 0.086720, 0.000000, 0.000000, 1.000000, },
              {0.742818, 1.000000, 0.127076, 0.132591, 0.123028, 0.012767, 0.012532, 0.000000, 0.017375, }}
},
// C5
{  525.75800, {0.22,
              {0.000000, 1.000000, 0.902633, 0.000000, 0.000000, 0.546067, 0.273848, 0.165969, 0.000000, 1.000000, },
              {1.000000, 0.952065, 0.209833, 0.145419, 0.153999, 0.038100, 0.061782, 0.026308, 0.000000, 0.010950, }}
},
// C5#
{  557.09200, {0.25,
              {0.000000, 1.000000, 1.000000, 0.459277, 1.000000, 0.128391, 0.490995, 0.000000, },
              {0.443332, 1.000000, 0.032761, 0.097356, 0.037776, 0.034402, 0.016249, 0.016535, }}
},
// D5
{  589.68900, {0.30,
              {0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, },
              {0.486039, 1.000000, 0.048903, 0.090611, 0.012278, 0.010149, }}
},
// D5#
{  623.97300, {0.30,
              {0.000000, 0.000000, 0.000000, 0.727659, 0.630439, 0.000000, 0.000000, },
              {1.000000, 0.634152, 0.000000, 0.098358, 0.027952, 0.000000, 0.010275, }}
},
// E5
{  661.35900, {0.30,
              {0.000000, 1.000000, 0.085378, 0.263783, 0.000000, 0.000000, 1.000000, 0.703376, 0.000000, 0.000000, 0.859382, },
              {0.510656, 1.000000, 0.037866, 0.137120, 0.022091, 0.011496, 0.025062, 0.010763, 0.000000, 0.000000, 0.010844, }}
},
// F5
{  699.93400, {0.15,
              {0.000000, 0.896721, 0.851269, 0.000000, 0.000000, 0.000000, 0.000000, 0.015816, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.248827, },
              {0.574150, 1.000000, 0.339015, 0.129839, 0.105608, 0.032199, 0.000000, 0.017142, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.012145, }}
},
// F5#
{  743.12400, {0.15,
              {0.000000, 0.891100, 0.000000, 0.213536, 0.000000, 1.000000, 0.000000, 0.125534, 0.535953, },
              {0.374055, 1.000000, 0.350298, 0.365048, 0.051860, 0.055696, 0.011215, 0.019488, 0.010645, }}
},
// G5
{  787.41500, {0.15,
              {0.000000, 0.768557, 0.576894, 0.089118, 0.287612, 0.231350, 0.000000, 1.000000, 0.000000, },
              {1.000000, 0.958088, 0.459752, 0.102034, 0.019914, 0.031096, 0.000000, 0.018014, 0.013451, }}
},
// G5#
{  834.23700, {0.20,
              {0.000000, 0.628649, 0.204374, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.113529, },
              {1.000000, 0.168715, 0.125072, 0.088768, 0.023883, 0.000000, 0.012386, 0.000000, 0.011735, }}
},
// A5
{  883.39000, {0.27,
              {0.000000, 0.000000, 0.617309, 0.577392, 0.000000, 0.587302, 0.884758, },
              {0.164986, 1.000000, 0.111940, 0.157719, 0.013662, 0.020707, 0.014294, }}
},
// A5#
{  935.61700, {0.25,
              {0.000000, 0.325743, 0.279960, 0.473329, 0.000000, 0.158150, 1.000000, 0.000000, 0.176026, },
              {1.000000, 0.765170, 0.304517, 0.090894, 0.015863, 0.021225, 0.011826, 0.000000, 0.010322, }}
},
// B5
{  990.84800, {0.25,
              {0.000000, 0.000000, 0.000000, 0.000000, 0.371177, 0.000000, 0.000000, },
              {1.000000, 0.345736, 0.000000, 0.164520, 0.022945, 0.016498, 0.027191, }}
},
// C6
{ 1052.08000, {0.16,
              {0.000000, 1.000000, 0.489395, 0.901466, 0.000000, 0.564716, 0.688017, 0.384432, 0.186622, 0.000000, 0.000000, 0.000000, 0.000000, },
              {1.000000, 0.648249, 0.254927, 0.207379, 0.012766, 0.034433, 0.024719, 0.035111, 0.010490, 0.000000, 0.024209, 0.000000, 0.011164, }}
},
// C6#
{ 1113.32000, {0.13,
              {0.000000, 0.545385, 0.000000, 0.255205, 0.055533, 0.195784, 0.226605, 0.402466, 0.000000, },
              {1.000000, 0.394530, 0.362940, 0.196300, 0.049123, 0.023201, 0.013357, 0.011119, 0.014265, }}
},
// D6
{ 1179.02000, {0.10,
              {0.000000, 0.000000, 0.000000, 0.735331, 0.000000, 0.000000, 0.000000, 0.010869, 0.020796, 0.000000, 0.948165, },
              {1.000000, 0.510868, 0.521743, 0.195312, 0.029235, 0.000000, 0.016882, 0.011065, 0.010463, 0.000000, 0.013931, }}
},
// D6#
{ 1248.84000, {0.09,
              {0.000000, 0.304583, 1.000000, 0.000000, 0.000000, 0.261126, },
              {1.000000, 0.503703, 0.092661, 0.126252, 0.014912, 0.025090, }}
},
// E6
{ 1321.55000, {0.08,
              {0.000000, 0.000000, 0.293211, 0.473708, 0.365399, 0.000000, 0.951362, 0.000000, 0.000000, },
              {1.000000, 0.671147, 0.266992, 0.105219, 0.017870, 0.017899, 0.019892, 0.026277, 0.015243, }}
},
// F6
{ 1400.53000, {0.09,
              {0.000000, 1.000000, 1.000000, 0.950506, 0.132275, 0.000000, },
              {1.000000, 0.297113, 0.332250, 0.122874, 0.018853, 0.012696, }}
},
// F6#
{ 1485.38000, {0.09,
              {0.000000, 0.520080, 1.000000, 0.067380, 0.000000, 0.000000, },
              {0.895249, 1.000000, 0.190910, 0.084265, 0.000000, 0.026533, }}
},
// G6
{ 1575.13000, {0.10,
              {0.000000, 0.420037, 1.000000, 0.497732, 0.000000, 0.000000, },
              {1.000000, 0.350544, 0.138616, 0.033539, 0.014901, 0.012713, }}
},
// G6#
{ 1668.84000, {0.11,
              {0.000000, 0.000000, 0.000000, 0.265256, 0.598529, 0.212701, 0.494673, },
              {1.000000, 0.440989, 0.031818, 0.032933, 0.016337, 0.021168, 0.016108, }}
},
// A6
{ 1767.02000, {0.13,
              {0.000000, 0.020647, 0.280440, 0.000000, 0.000000, 0.000000, 0.000000, },
              {1.000000, 0.689164, 0.207594, 0.107791, 0.011286, 0.028248, 0.014659, }}
},
// B6 ??
// C7
{ 2100.50000, {0.14,
              {0.000000, 0.718946, 0.077632, 0.763894, 0.076217, 0.000000, 1.000000, 0.000000, 0.513299, 0.371054, },
              {1.000000, 0.964414, 0.780531, 0.025363, 0.042657, 0.016619, 0.012577, 0.012675, 0.014242, 0.012046, }}
},
// C7#
{ 2226.47000, {0.12,
              {0.000000, 0.794569, 0.812474, 0.646604, 0.420340, },
              {1.000000, 0.313442, 0.025512, 0.051774, 0.013735, }}
},
// D7
{ 2358.14000, {0.10,
              {0.000000, 0.236327, 0.000000, 1.000000, 0.077167, 0.180553, },
              {0.327596, 1.000000, 0.015345, 0.020431, 0.038691, 0.011876, }}
},
// D7#
{ 2500.20000, {0.09,
              {0.000000, 1.000000, 0.465360, 0.596163, 0.000000, 0.679636, 0.155637, },
              {0.391603, 1.000000, 0.096775, 0.154982, 0.019778, 0.012725, 0.013184, }}
},
// E7
{ 2644.75000, {0.08,
              {0.000000, 0.165333, 0.003047, 0.488385, 0.000000, 0.000000, },
              {1.000000, 0.530780, 0.109227, 0.012611, 0.000000, 0.010332, }}
},
// F7
{ 2804.32000, {0.06,
              {0.000000, 0.940406, 0.774100, 0.000000, 0.000000, },
              {1.000000, 0.900730, 0.044079, 0.065749, 0.032851, }}
},
// F7#
{ 2970.49000, {0.06,
              {0.000000, 0.865605, 0.490331, 0.755248, 0.000000, 0.579930, 0.594048, },
              {0.791288, 1.000000, 0.492588, 0.145057, 0.018775, 0.026327, 0.024794, }}
},
// G7
{ 3147.59000, {0.07,
              {0.000000, 0.299683, 0.000000, 0.000000, 0.000000, 0.551083, 0.000000, },
              {1.000000, 0.580715, 0.049265, 0.031220, 0.000000, 0.015356, 0.019788, }}
},
// G7#
{ 3334.10000, {0.10,
              {0.000000, 0.132434, 0.000000, 0.532003, },
              {0.450382, 1.000000, 0.015618, 0.013524, }}
},
// A7
{ 3533.88000, {0.12,
              {0.000000, 0.000000, 0.124131, 0.356764, },
              {1.000000, 0.771905, 0.225805, 0.022204, }}
},
// B7 ??

};

const Parameters & get_nearest_params(float64 frequency)
{
    auto lower = principal_stops.lower_bound(frequency);
    auto upper = lower; ++upper;
    if (lower == principal_stops.end()) return principal_stops.begin()->second;
    if (upper == principal_stops.end()) return lower->second;
    auto dist_to_lower = std::abs(lower->first - frequency);
    auto dist_to_upper = std::abs(upper->first - frequency);
    return (dist_to_upper < dist_to_lower) ? upper->second : lower->second;
}

} // Anonymous namespace.



AudioStream
OrganPipe::
play(
    const float64 & duration,
    const float64 & frequency,
    const float64 & pan)
{
    M_ASSERT_VALUE(duration, >, 0);

    const auto & params = get_nearest_params(frequency);
    const auto & scale = params.scale;
    const auto & phases = params.phases;
    const auto & weights = params.weights;

    float64 pan_left = pan;
    float64 pan_right = 1.0 - pan_left;

    Buffer dclick;
    Buffer amp1;
    Buffer amp2;
    Buffer amp3;
    Buffer amp4;

    if(duration > 0.002)
    {
        dclick
            << _sine->drawLine(0.001, 0.0, scale)
            << _sine->drawLine(duration - 0.002, scale, scale)
            << _sine->drawLine(0.001, scale, 0.0);
    }
    else
    {
        dclick << _sine->drawLine(duration, scale, scale);
    }

    if(duration > 0.02)
    {
        amp1
            << _sine->drawLine(0.01, 0.0, 1.0)
            << _sine->drawLine(duration - 0.02, 1.0, 1.0)
            << _sine->drawLine(0.01, 1.0, 0.0);
    }
    else
    {
        amp1 = _sine->drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.16)
    {
        amp2
            << _sine->drawLine(0.05, 0.0, 1.0)
            << _sine->drawLine(0.1, 1.0, 0.7)
            << _sine->drawLine(duration - 0.16, 0.7, 0.7)
            << _sine->drawLine(0.01, 0.7, 0.0);
    }
    else
    {
        auto third = duration / 3.0;
        amp2
            << _sine->drawLine(third, 0.0, 1.0)
            << _sine->drawLine(duration - third, 1.0, 0.0);
    }

    if(duration > 0.08)
    {
        amp3
            << _sine->drawLine(0.03, 0.0, 1.0)
            << _sine->drawLine(0.05, 1.0, 0.0)
            << _sine->drawLine(duration - 0.08, 0.0, 0.0);
    }
    else
    {
        amp3 = _sine->drawLine(duration, 1.0, 1.0);
    }

    if(duration > 0.21)
    {
        amp4
            << _sine->drawLine(0.1, 0.0, 0.03)
            << _sine->drawLine(0.1, 0.03, 0.01)
            << _sine->drawLine(duration -0.21, 0.01, 0.01)
            << _sine->drawLine(0.01, 0.01, 0.0);
    }
    else
    {
        auto half = duration / 2.0;
        amp4
            << _sine->drawLine(half, 0.0, 0.3)
            << _sine->drawLine(duration - half, 0.3, 0.0);
    }

    Buffer out = _sine->silence(duration);

    for (std::size_t i = 0 ; i < weights.size(); ++i)
    {
        const auto f = (i + 1.0) * frequency;
        const auto & p = phases[i];
        const auto & w = weights[i];

        if (i == 0)
        {
            out += amp1 * _sine->generate2(duration, f, p) * w;
        }
        else
        {
            out += amp2 * _sine->generate2(duration, f, p) * w;
        }
    }

    AudioStream y(sample_rate_, 2);

    y[0] = out * dclick * pan_left;
    y[1] = out * dclick * pan_right;

    return y;
}


AudioStream
OrganPipe::
play()
{
    AudioStream y(sample_rate_, 2);

    for (const auto & pair: principal_stops)
    {
        const auto freq = pair.first;

        std::string fout = "organ-freq-" + std::to_string(static_cast<int>(freq)) + ".wav";

        AudioStream out(sample_rate_, 2);

        out << play(1.0, freq);
        out >> fout.c_str();
    }


    // Organ Intro to Bach's Toccata & Fugue in Dminor
    //     Start  Dur  Amp    Fqc   Pan  OutCh1  OutCh2
    // i4  26     .12  200    7.09  .9   1       2
    // i4  +      .1   300    7.07  .8   1       2
    // i4  .      .8   400    7.09  .7   1       2
    // i4  27.2   .16  500    7.07  .6   1       2
    // i4  +      .14  600    7.05  .5   1       2
    // i4  .      .12  700    7.04  .4   1       2
    // i4  .      .12  800    7.02  .3   1       2
    // i4  .      .56  900    7.01  .4   1       2
    // i4  .     1.2   1200   7.02  .5   1       2

    y << play(0.12, 219.98, 0.9)
      << play(0.10, 195.99, 0.8)
      << play(0.80, 219.98, 0.7);

    // Use AudioStream::add to insert the precise delay.
    y.add(play(0.16, 195.99, 0.6), 1.20f);

    y << play(0.14, 174.61, 0.5)
      << play(0.12, 164.80, 0.4)
      << play(0.12, 146.83, 0.3)
      << play(0.56, 138.58, 0.4)
      << play(1.20, 146.83, 0.5);

    // i4  29.8   .12  1600   6.09  .5
    // i4  +      .1   .      6.07  .5
    // i4  .      .8   .      6.09  .5
    // i4  31     .3   .      6.05  .5
    // i4  +      .3   .      6.07  .5
    // i4  .      .3   .      6.01  .5
    // i4  .     1.2   .      6.02  .5

    y.add(play(0.12, 109.99), 2.80f);

    y << play(0.10,  97.99)
      << play(0.80, 109.99);

    y.add(play(0.30,  87.30), 5.00f);

    y << play(0.30,  97.99)
      << play(0.30,  69.29)
      << play(1.20,  73.41);

    // i4  33.2   .12  3000   5.09  .5
    // i4  +      .1   .      5.07  .5
    // i4  .      .8   .      5.09  .5
    // i4  34.4   .16  .      5.07  .5
    // i4  +      .14  .      5.05  .5
    // i4  .      .12  .      5.04  .5
    // i4  .      .12  .      5.02  .5
    // i4  .      .56  .      5.01  .5
    // i4  .     1.2   .      5.02  .5

    y.add(play(0.12, 55.00), 7.20f);

    y << play(0.10, 49.00)
      << play(0.80, 55.00);

    y.add(play(0.16, 49.00), 8.40f);

    y << play(0.14, 43.65)
      << play(0.12, 41.20)
      << play(0.12, 36.71)
      << play(0.56, 34.65)
      << play(1.20, 36.71);

    // i4  36.5  2.0   .      5.01  .5
    // i4  36.7  1.8   .      5.04  .5
    // i4  36.9  1.6   .      5.07  .5
    // i4  37.1  1.4   .      5.10  .5
    // i4  37.3  1.2   .      6.01  .5
    // i4  38.7  3.2   .      5.02  .5
    // i4  38.7  3.2   .      6.02  .5
    // i4  38.7  0.8   .      5.07  .5
    // i4  +     0.8   .      5.09  .5
    // i4  .     1.6   .      5.06  .5

    y.add(play(2.00, 34.65), 10.5f);
    y.add(play(1.80, 41.20), 10.7f);
    y.add(play(1.60, 49.00), 10.9f);
    y.add(play(1.40, 58.27), 11.1f);
    y.add(play(1.20, 69.29), 11.3f);
    y.add(play(3.20, 36.71), 12.7f);
    y.add(play(3.20, 73.41), 12.7f);
    y.add(play(0.08, 49.00), 12.7f);

    for(uint32 i = 0 ; i < sample_rate_ * 0.3; ++i) y << 0.0;

    y.normalize();

    return y;
}


// :mode=c++: jEdit modeline
