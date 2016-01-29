//-----------------------------------------------------------------------------
//
//  $Id: Design.cc 932 2015-12-12 17:13:47Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2009 to Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
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

#include <algorithm>
#include <cmath>
#include <iomanip>

#include <Nsound/biquad/Design.h>

namespace Nsound
{
namespace biquad
{

//~#define DEBUG_OUT

#ifdef DEBUG_OUT
    #define dout std::cout
#else
    #define dout if(false) std::cout
#endif


// support types and operators

typedef std::vector<bool> BoolVector;
typedef std::vector<float64> Vector;
typedef std::vector<Vector> Matrix2D;

Vector operator * (const Vector & lhs, const Vector & rhs);
Vector operator * (const Vector & lhs, float64 rhs);
Vector operator * (float64 lhs, const Vector & rhs);

BoolVector operator == (const Vector & lhs, float64 rhs);
BoolVector operator == (float64 lhs, const Vector & rhs) { return rhs == lhs; }

BoolVector operator != (const Vector & lhs, float64 rhs);
BoolVector operator != (float64 lhs, const Vector & rhs) { return rhs != lhs; }

BoolVector operator && (const BoolVector & lhs, const BoolVector & rhs);
BoolVector operator || (const BoolVector & lhs, const BoolVector & rhs);

Matrix2D zeros(uint32 m, uint32 n);
Vector column(const Matrix2D &, uint32);


struct Shape
{
    const uint32 m;
    const uint32 n;
};


Shape shape(const Matrix2D &);

bool at_least_row_vector(const Matrix2D &);

Matrix2D transpose(const Matrix2D &);

struct BilinearTransformResult
{
    Matrix2D b;
    Matrix2D a;
    Matrix2D bhat;
    Matrix2D ahat;
};

std::ostream & operator << (std::ostream & out, const Matrix2D & in);

template <class T>
std::ostream & operator << (std::ostream & out, const std::vector<T> & in)
{
    out << "[";

    for(const auto & e : in)
    {
        out << e << ", ";
    }

    return out << "]";
}


BilinearTransformResult
bilinear_transform(const Matrix2D & ba, const Matrix2D & aa, float64 w0);

Vector cas2dir(const Matrix2D & matrix);

Vector convolve(const Vector & x, const Vector & h);


//-----------------------------------------------------------------------------
//
// band_edge.m - calculate left and right bandedge frequencies from bilinear transformation

BandEdge::
BandEdge() : _lo_hz(0.0), _hi_hz(0.0) {}


BandEdge::
BandEdge(
    float64 sample_rate,
    float64 freq_center_hz,
    float64 bandwidth_hz)
    :
    _lo_hz(0.0),
    _hi_hz(0.0)
{
    const float64 hz_to_radians = 2 * M_PI / sample_rate;
    const float64 radians_to_hz = 1.0 / hz_to_radians;

    float64 wc = freq_center_hz * hz_to_radians;
    float64 wb = bandwidth_hz * hz_to_radians;

    float64 wwbb = std::tan(wb / 2.0);
    float64 c0 = std::cos(wc);
    float64 s0 = std::sin(wc);

    float64 temp0 = wwbb * std::sqrt(wwbb*wwbb + s0 * s0);
    float64 temp1 = 1.0 + wwbb * wwbb;

    _lo_hz = radians_to_hz * std::acos((c0 + temp0) / temp1);
    _hi_hz = radians_to_hz * std::acos((c0 - temp0) / temp1);
}


//-----------------------------------------------------------------------------
//
//  hpeq.m - high-order digital parametric equalizer design
//
//  Usage: [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,type,Gs,tol)
//
//
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw)          Butterworth
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,0)        Butterworth
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,1)        Chebyshev-1
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,2)        Chebyshev-2
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,3,Gs,tol) elliptic, e.g., tol = 1e-8
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,3,Gs,M)   elliptic, tol = M = Landen iterations, e.g., M=5
//         [B,A,Bh,Ah] = hpeq(N,G0,G,GB,w0,Dw,3,Gs)     elliptic, default tol = eps
//
//  N  = analog filter order
//  G0 = reference gain (all gains must be in dB, enter -Inf to get 0 in absolute units)
//  G  = peak/cut gain
//  GB = bandwidth gain
//  w0 = peak/cut center frequency in units of radians/sample, i.e., w0=2*pi*f0/fs
//  Dw = bandwidth in radians/sample, (if w0=pi, Dw = cutoff freq measured from Nyquist)
//  type = 0,1,2,3, for Butterworth, Chebyshev-1, Chebyshev-2, and elliptic (default is type=0)
//  Gs = stopband gain, for elliptic case only
//  tol = tolerance for elliptic case, e.g., tol = 1e-10, default value is tol = eps = 2.22e-16
//
//  B,A   = rows are the numerator and denominator 4th order section coefficients of the equalizer
//  Bh,Ah = rows are the numerator and denominator 2nd order coefficients of the lowpass shelving filter
//
//  notes: G,GB,G0 are in dB and converted internally to absolute units, e.g. G => 10^(G/20)
//
//         gains must satisfy: G0<Gs<GB<G (boost), or, G0>Gs>GB>G (cut)  (exchange roles of Gs,GB for Cheby-2)
//
//         w0 = 2*pi*f0/fs, Dw = 2*pi*Df/fs, with f0,Df,fs in Hz
//
//         B,A have size (L+1)x5, and Bh,Ah, size (L+1)x3, where L=floor(N/2)
//         when N is even, the first row is just a gain factor
//
//         left and right bandedge frequencies: [w1,w2] = bandedge(w0,Dw)
//         for the stopband in the elliptic case: [w1s,w2s] = bandedge(w0,Dws), where
//         k = ellipdeg(N,k1,tol)
//         WB = tan(Dw/2)
//         Ws = WB/k
//         Dws = 2*atan(Ws)
//
//         Run the functions HPEQEX0, HPEQEX1, HPEQEQ2 to generate some examples
//
//  see also, BLT, ELLIDPEG, ELLIPK, ASNE, CDE, BANDEDGE, HPEQBW, OCTBW
//
//-----------------------------------------------------------------------------

BiquadKernel
hpeq_design(
    float64    sample_rate,
    uint32     order,
    float64    freq_center_hz,
    float64    bandwidth_hz,
    float64    gain_reference_db,
    float64    gain_freq_center_db,
    float64    gain_bandwidth_db,
    DesignType type)
{
    M_ASSERT_MSG(order > 0, "order must be > 0 (order == 0)");

    M_ASSERT_MSG(
        type == BUTTERWORTH,
        "Currently design type BUTTERWORTH is implemented, sorry");

    M_ASSERT_MSG(
        sample_rate > 0.0,
        "sample_rate must be > 0.0 (" << sample_rate << " < 0.0)");

    M_ASSERT_MSG(
        bandwidth_hz > 0.0,
        "bandwidth_hz must be > 0.0 (" << bandwidth_hz << " <= 0.0)");

    // keep order even if > 1

    if(order > 1)
    {
        order += order % 2;
    }

    dout << "fc = " << freq_center_hz * 2 * M_PI / sample_rate << "\n";
    dout << "wb = " << bandwidth_hz * 2 * M_PI / sample_rate << "\n";
    dout << "G0 = " << gain_reference_db << "\n";
    dout << "G = " << gain_freq_center_db << "\n";
    dout << "GB = " << gain_bandwidth_db << "\n";

    // convert to linear gain

    float64 gref = std::pow(10.0, gain_reference_db / 20.0);
    float64 gfc  = std::pow(10.0, gain_freq_center_db / 20.0);
    float64 gbw  = std::pow(10.0, gain_bandwidth_db / 20.0);

    // no filtering if gfc ~= gref
    if(std::abs(gfc - gref) < 1e-7)
    {
        BiquadKernel bck =
        {
            // b     a
            {gref}, {1},
        };

        return bck;
    }

    uint32 r = order % 2;
    uint32 l = (order - r) / 2;

    float64 hz_to_radians = 2 * M_PI / sample_rate;

    float64 w0 = freq_center_hz * hz_to_radians;

//~    float64 c0 = std::cos(w0);

//~    // special cases
//~    if(w0 == 0.0 || w0 == -0.0)
//~    {
//~        c0 = 1.0;
//~    }

//~    else
//~    if(w0 == M_PI / 2.0)
//~    {
//~        c0 = 0.0;
//~    }

//~    else
//~    if(w0 == M_PI)
//~    {
//~        c0 = -1.0;
//~    }

    float64 wb = std::tan(bandwidth_hz * hz_to_radians / 2.0);

    float64 gbw2 = gbw * gbw;

    float64 numerator   = gfc * gfc - gbw2;
    float64 denominator = gbw2      - gref * gref;

    float64 e = 3000.0;

    if(std::abs(denominator) > 0.01)
    {
        e = std::sqrt(std::abs(numerator) / std::abs(denominator));
    }

    // adjust gain
    gfc = std::pow(gfc, 1.0 / order);
    gref = std::pow(gref, 1.0 / order);

    float64 a = 0.0;
    float64 b = 0.0;

    switch(type)
    {
        case BUTTERWORTH:
        {
            a = std::pow(e, 1.0 / order);
            b = gref * a;
            break;
        }

        default:
        {
            M_THROW("Only type BUTTERWORTH is currently implemented");
        }
    }

    Matrix2D ba;
    Matrix2D aa;

    dout << "r = " << r << "\n";
    dout << "L = " << l << "\n";

    if(r == 0)
    {
        if(type != ELLIPTIC)
        {
            ba = {{1, 0, 0}};
            aa = {{1, 0, 0}};
        }

        // elliptic case
        else
        {
            ba = {{gbw, 0, 0}};
            aa = {{1, 0, 0}};
        }
    }

    else
    if(r == 1)
    {
        switch(type)
        {
            case BUTTERWORTH:  // fall through
            case CHEBYSHEV_2:
            {
                ba = {{gfc * wb, b, 0}};
                aa = {{wb,       a, 0}};
                break;
            }

            case CHEBYSHEV_1:
            {
                ba = {{b * wb, gref, 0}};
                aa = {{a * wb,    1, 0}};
                break;
            }

            default:
            {
                M_THROW("oops, case not implmented!");
            }
        }
    }

    if(l > 0)
    {
        std::vector<float64> x;
        std::vector<float64> ux;
        std::vector<float64> ci;
        std::vector<float64> si;
        std::vector<float64> v;

        float64 n = static_cast<float64>(order);

        for(uint32 i = 1; i < l + 1; ++i)
        {
            x.push_back(static_cast<float64>(i));
            ux.push_back(static_cast<float64>(2 * i - 1) / n);

            ci.push_back(std::cos(M_PI * ux[i-1] / 2.0));
            si.push_back(std::sin(M_PI * ux[i-1] / 2.0));
            v.push_back(1.0);
        }

        dout << "------------------------------------------------\n";
        dout << "N = " << order << "\n";
        dout << "r = " << r << "\n";
        dout << "L = " << l << "\n";
        dout << "i = ";

        for(auto i : x)
        {
            dout << i << ", ";
        }

        dout << "\nui = ";

        for(auto i : ux)
        {
            dout << i << ", ";
        }

        dout << "\nci = ";

        for(auto i : ci)
        {
            dout << i << ", ";
        }

        dout << "\nsi = ";

        for(auto i : si)
        {
            dout << i << ", ";
        }

        dout << "\nv = ";

        for(auto i : v)
        {
            dout << i << ", ";
        }

        dout << "\n";
        dout << "gfc = " << gfc << "\n";
        dout << "wb = " << std::setprecision(15) << wb << "\n";
        dout << "b = " << b << "\n";
        dout << "a = " << a << "\n";

        Matrix2D temp_ba;
        Matrix2D temp_aa;

        switch(type)
        {
            case BUTTERWORTH:
            {
                temp_ba.push_back(gfc * gfc * wb * wb * v);
                temp_ba.push_back(2 * gfc * b * si * wb);
                temp_ba.push_back(b * b * v);

                temp_aa.push_back(wb * wb * v);
                temp_aa.push_back(2 * a * si * wb);
                temp_aa.push_back(a * a * v);

                break;
            }

            default:
            {
                M_THROW("oops, case not implemented");
            }
        }

        temp_ba = transpose(temp_ba);
        temp_aa = transpose(temp_aa);

        for(auto & row : temp_ba) ba.push_back(row);
        for(auto & row : temp_aa) aa.push_back(row);

        uint32 i = 0;

        for(auto & bbb : ba)
        {
            dout << "ba" << i++ << " : ";

            for(auto & xxx : bbb)
            {
                dout << xxx << ", ";
            }

            dout << "\n";
        }

        i = 0;

        for(auto & aaa : aa)
        {
            dout << "aa" << i++ << " : ";

            for(auto & xxx : aaa)
            {
                dout << xxx << ", ";
            }

            dout << "\n";
        }
    }

    dout << "f0 = " << freq_center_hz << "\n";

    // bilinear transform

    auto blt = bilinear_transform(ba, aa, w0);

    // convert form cascade to direct form 1

    BiquadKernel bk = {cas2dir(blt.b), cas2dir(blt.a)};

    dout << "--------------------------------------------------\n";
    dout << "director form 1\n";
    dout << "bk.b = " << bk._b << "\n";
    dout << "bk.a = " << bk._a << "\n";

    return bk;
}


//-----------------------------------------------------------------------------
// support functions


//-----------------------------------------------------------------------------
//
//  blt.m - bilinear transformation of analog second-order sections
//
//  Usage: [B,A,Bhat,Ahat] = blt(Ba,Aa,w0);
//
//  Ba,Aa = Kx3 matrices of analog numerator and denominator coefficients (K sections)
//  w0    = center frequency in radians/sample
//
//  B,A = Kx5 matrices of numerator and denominator coefficients (4th-order sections in z)
//  Bhat,Ahat = Kx3 matrices of 2nd-order sections in the variable zhat
//
//  notes: It implements the two-stage bilinear transformation:
//                        s    -->    zhat    -->    z
//                   LP_analog --> LP_digital --> BP_digital
//
//         s = (zhat-1)/(zhat+1) = (z^2 - 2*c0*z + 1)/(z^2 - 1), with zhat = z*(c0-z)/(1-c0*z)
//
//         c0 = cos(w0), where w0 = 2*pi*f0/fs = center frequency in radians/sample
//
//         (B0 + B1*s + B2*s^2)/(A0 + A1*s + A2*s^2) =
//         (b0h + b1h*zhat^-1 + b2h*zhat^-2)/(1 + a1h*zhat^-1 + a2h*zhat^-2) =
//         (b0 + b1*z^-1 + b2*z^-2 + b3*z^-3 + b4*z^-4)/(1 + a1*z^-1 + a2*z^-2 + a3*z^-3 + a4*z^-4)
//
//         column-wise, the input and output matrices have the forms:
//         Ba = [B0,B1,B2], Bhat = [b0h, b1h, b2h], B = [b0,b1,b2,b3,b4]
//         Aa = [A0,A1,A2], Ahat = [1,   a1h, a2h], A = [1, a1,a2,a3,a4]
//
//-----------------------------------------------------------------------------

BilinearTransformResult
bilinear_transform(const Matrix2D & ba, const Matrix2D & aa, float64 w0)
{
    M_ASSERT_MSG(at_least_row_vector(ba), "ba must be at least a row vector");
    M_ASSERT_MSG(at_least_row_vector(aa), "aa must be at least a row vector");

    Shape shp_bb = shape(ba);

    auto n_sections = shp_bb.m;

    auto B = zeros(n_sections, 5);
    auto A = B;

    auto Bhat = zeros(n_sections, 3);
    auto Ahat = Bhat;

    // simplify notation

    auto B0 = column(ba, 0);
    auto B1 = column(ba, 1);
    auto B2 = column(ba, 2);

    auto A0 = column(aa, 0);
    auto A1 = column(aa, 1);
    auto A2 = column(aa, 2);

    auto c0 = 0.0;

    //-------------------------------------------------------------------------
    // make sure special cases are computed exactly

    if(w0 == 0.0)
    {
        c0 = 1.0;
    }

    else
    if(w0 == M_PI)
    {
        c0 = -1.0;
    }

    else
    if(w0 == M_PI / 2.0)
    {
        c0 = 0.0;
    }

    else
    {
        c0 = std::cos(w0);
    }

    dout << "w0 = " << w0 << "\n";
    dout << "c0 = " << c0 << "\n";

    //-------------------------------------------------------------------------
    // find 0th-order sections (i.e., gain sections)

    dout << "---------------------------------------------------------\n";
    dout << "find 0th-order sections (i.e., gain sections)\n";

    auto bv =
        B1 == 0.0 &&
        A1 == 0.0 &&
        B2 == 0.0 &&
        A2 == 0.0;

    auto tmp_idx = std::find(bv.begin(), bv.end(), true);

    auto idx = 0;

    if(tmp_idx != bv.end())
    {
        idx = tmp_idx - bv.begin();
    }

    dout << "idx = " << idx << "\n";

    Bhat[idx][0] = B0[idx] / A0[idx];
    Ahat[idx][0] = 1.0;

    dout << "Bhat = " << Bhat << "\n";
    dout << "Ahat = " << Ahat << "\n";

    B[idx][0] = Bhat[idx][0];
    A[idx][0] = 1.0;

    dout << "B = " << B << "\n";
    dout << "A = " << A << "\n";

    //-------------------------------------------------------------------------
    // find 1st-order analog sections

    dout << "---------------------------------------------------------\n";
    dout << "find 1st-order analog sections\n";

    auto bv0 = B1 != 0.0 || A1 != 0.0;
    auto bv1 = B2 == 0.0 && A2 == 0.0;

    bv = bv0 && bv1;

    dout << "bv = " << bv << "\n";

    for(uint32 j = 0; j < bv.size(); ++j)
    {
        if(!bv[j]) continue;

        auto D = A0[j] + A1[j];

        Bhat[j][0] = (B0[j] + B1[j]) / D;
        Bhat[j][1] = (B0[j] - B1[j]) / D;
        Ahat[j][0] = 1.0;
        Ahat[j][
        1] = (A0[j] - A1[j]) / D;

        B[j][0] = Bhat[j][0];
        B[j][1] = c0 * (Bhat[j][1] - Bhat[j][0]);
        B[j][2] = -Bhat[j][1];
        A[j][0] = 1.0;
        A[j][1] = c0 * (Ahat[j][1] - 1.0);
        A[j][2] = -Ahat[j][1];
    }

    dout << "Bhat = " << Bhat << "\n";
    dout << "Ahat = " << Ahat << "\n";

    dout << "B = " << B << "\n";
    dout << "A = " << A << "\n";

    //-------------------------------------------------------------------------
    // find 2nd-order analog sections

    dout << "---------------------------------------------------------\n";
    dout << "find 2nd-order analog sections\n";

    bv = B2 != 0.0 || A2 != 0.0;

    dout << "bv = " << bv << "\n";

    for(uint32 j = 0; j < bv.size(); ++j)
    {
        if(!bv[j]) continue;

        auto D = A0[j] + A1[j] + A2[j];

        Bhat[j][0] = (B0[j] + B1[j] + B2[j]) / D;
        Bhat[j][1] = 2 * (B0[j] - B2[j]) / D;
        Bhat[j][2] = (B0[j] - B1[j] + B2[j]) / D;

        Ahat[j][0] = 1.0;
        Ahat[j][1] = 2 * (A0[j] - A2[j]) / D;
        Ahat[j][2] = (A0[j] - A1[j] + A2[j]) / D;

        B[j][0] = Bhat[j][0];
        B[j][1] = c0 * (Bhat[j][1] - 2 * Bhat[j][0]);
        B[j][2] = (Bhat[j][0] - Bhat[j][1] + Bhat[j][2]) * (c0 * c0) - Bhat[j][1];
        B[j][3] = c0 * (Bhat[j][1] - 2 * Bhat[j][2]);
        B[j][4] = Bhat[j][2];

        A[j][0] = 1.0;
        A[j][1] = c0 * (Ahat[j][1] - 2.0);
        A[j][2] = (1.0 - Ahat[j][1] + Ahat[j][2]) * (c0 * c0) - Ahat[j][1];
        A[j][3] = c0 * (Ahat[j][1] - 2 * Ahat[j][2]);
        A[j][4] = Ahat[j][2];
    }

    dout << "Bhat = " << Bhat << "\n";
    dout << "Ahat = " << Ahat << "\n";

    dout << "B = " << B << "\n";
    dout << "A = " << A << "\n";

    //-------------------------------------------------------------------------
    // LP or HP shelving filter

    if(std::abs(c0) == 1.0)
    {
        B = Bhat;     // B,A are second-order
        A = Ahat;

        for(auto & row : B) row[1] *= c0;   // change sign if w0=pi
        for(auto & row : A) row[1] *= c0;

        // make them k x 5

        for(auto & row : B) {row.push_back(0.0); row.push_back(0.0);}
        for(auto & row : A) {row.push_back(0.0); row.push_back(0.0);}

        dout << "-----------------------------------------------------\n";
        dout << "LP or HP shelving filter\n";
        dout << "B = " << B << "\n";
        dout << "A = " << A << "\n";
    }

    return {B, A, Bhat, Ahat};
}


Vector operator * (const Vector & lhs, const Vector & rhs)
{
    M_ASSERT_MSG(
        lhs.size() == rhs.size(),
        "Size mismatch (" << lhs.size() << " != " << rhs.size() << ")");

    Vector out;

    std::transform(
        lhs.begin(), lhs.end(),
        rhs.begin(), std::back_inserter(out),
        std::multiplies<float64>() );

    return out;
}


Vector operator * (const Vector & lhs, float64 rhs)
{
    Vector out(lhs);

    for(auto & x : out)
    {
        x *= rhs;
    }

    return out;
}


Vector operator * (float64 lhs, const Vector & rhs)
{
    Vector out(rhs);

    for(auto & x : out)
    {
        x *= lhs;
    }

    return out;
}


Shape shape(const Matrix2D & in)
{
    uint32 m = in.size();

    M_ASSERT_VALUE(m, >, 0);

    uint32 n = in[0].size();

    // Assert that all rows have n elements

    if(true) // enable while debugging
    {
        for(const auto & row : in)
        {
            M_ASSERT_MSG(
                row.size() == n,
                "invalid Matrix2D, row length ( " << row.size() << " != " << n
                << ")");
        }
    }

    return {m, n};
}


bool at_least_row_vector(const Matrix2D & in)
{
    Shape shp = shape(in);

    if(shp.m > 1 && shp.n == 1)
    {
        return false;
    }

    return true;
}


Matrix2D transpose(const Matrix2D & in)
{
    Matrix2D out;

    const auto shp = shape(in);

    // add rows to out

    for(uint32 i = 0; i < shp.n; ++i)
    {
        out.push_back(Vector());
    }

    // transpose

    for(uint32 n = 0; n < shp.n; ++n)
    {
        for(uint32 m = 0; m < shp.m; ++m)
        {
            out[n].push_back(in[m][n]);
        }
    }

    return out;
}


Matrix2D zeros(uint32 m, uint32 n)
{
    Matrix2D out;

    for(uint32 i = 0 ; i < m; ++i)
    {
        out.push_back(Vector(n, 0));
    }

    return out;
}


Vector column(const Matrix2D & in, uint32 n)
{
    Vector col;

    for(auto & row : in)
    {
        col.push_back(row[n]);
    }

    return col;
}


BoolVector operator == (const Vector & lhs, float64 rhs)
{
    BoolVector out;

    for(auto & l : lhs) out.push_back(l == rhs);

    return out;
}


BoolVector operator != (const Vector & lhs, float64 rhs)
{
    BoolVector out;

    for(auto & l : lhs) out.push_back(l != rhs);

    return out;
}


BoolVector operator && (const BoolVector & lhs, const BoolVector & rhs)
{
    BoolVector out;

    auto N = lhs.size();

    M_ASSERT_MSG(N == rhs.size(), "size mismatch (" << N << " != " << rhs.size() << ")");

    for(uint32 i = 0; i < N; ++i)
    {
        out.push_back(lhs[i] && rhs[i]);
    }

    return out;
}


BoolVector operator || (const BoolVector & lhs, const BoolVector & rhs)
{
    BoolVector out;

    auto N = lhs.size();

    M_ASSERT_MSG(N == rhs.size(), "size mismatch (" << N << " != " << rhs.size() << ")");

    for(uint32 i = 0; i < N; ++i)
    {
        out.push_back(lhs[i] || rhs[i]);
    }

    return out;
}


std::ostream & operator << (std::ostream & out, const Matrix2D & rhs)
{
    out << "[\n";

    for(const auto & row : rhs)
    {
        out << "    [";

        for(const auto v : row)
        {
            out << v << ", ";
        }

        out << "]\n";
    }

    return out << "]";
}


Vector
cas2dir(const Matrix2D & matrix)
{
    const Shape s = shape(matrix);

    M_ASSERT_MSG(s.m > 0, "matrix must have at leats 1 row");

    Vector out = {1.0};

    for(auto & row : matrix)
    {
        out = convolve(out, row);
    }

    return out;
}


Vector
convolve(const Vector & x, const Vector & h)
{
    Vector y(x.size() + h.size() - 1, 0.0);

    for(auto i = 0u; i < x.size(); ++i)
    {
        for(auto j = 0u; j < h.size(); ++j)
        {
            y[i + j] += x[i] * h[j];
        }
    }

    return y;
}


} // namespace
} // namespace