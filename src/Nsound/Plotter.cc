//-----------------------------------------------------------------------------
//
//  $Id: Plotter.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>

//#include <numpy/arrayobject.h>

#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cout;
using std::cerr;

Plotter::PylabTable Plotter::table_;

#define PRINT_LINE __FILE__ << ":" << __LINE__ << ": "

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

Plotter::PlotterState Nsound::Plotter::state_ = Plotter::BOOTING;
boolean Plotter::hold_is_on_ = false;
boolean Plotter::grid_is_on_ = true;

float64 Plotter::xmin_ =  1e300;
float64 Plotter::xmax_ = -1e300;
float64 Plotter::ymin_ =  1e300;
float64 Plotter::ymax_ = -1e300;

int32 Plotter::count_ = 0;

// Statically allocate one plotter object so show() works propperly.
static Plotter g_plotter;

#ifndef NSOUND_PLATFORM_OS_WINDOWS
	#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

//-----------------------------------------------------------------------------
Axes::
Axes(PyObject * axes_ptr)
    : axes_ptr_(axes_ptr)
{
}

Axes::
Axes(const Axes & rhs)
    : axes_ptr_(rhs.axes_ptr_)
{
}

Axes::
~Axes()
{
}

Axes &
Axes::
operator=(const Axes & rhs)
{
    axes_ptr_ = rhs.axes_ptr_;
    return *this;
}

#define M_CHECK_PY_PTR_RETURN( a, msg ) \
	if( PyErr_Occurred() || (a) == NULL) \
	{                                   \
		PyErr_Print();                  \
		M_THROW( msg );                 \
		return;                         \
	}

#define M_CHECK_PY_PTR( a, msg ) \
	if( PyErr_Occurred() || (a) == NULL) \
	{                            \
		PyErr_Print();           \
		M_THROW( msg );          \
	}

//-----------------------------------------------------------------------------
Plotter::
Plotter()
{
    #ifdef NSOUND_C_PYLAB

        ++count_;

        if(Plotter::state_ != BOOTING)
        {
            return;
        }

        #ifndef NSOUND_IN_PYTHON_MODULE

            Py_Initialize();

            // Initialize/import the Numeric::arrayobject module
//            import_array();

            if(PyErr_Occurred())
            {
                PyErr_Print();
                M_THROW("Py_Initialize() failed");
                return;
            }

        #endif

        //---------------------------------------------------------------------
        // importing modules

        PyObject * matplotlib = PyImport_ImportModule("matplotlib");

        M_CHECK_PY_PTR_RETURN( matplotlib, "import matplotlib failed." );

        PyObject * pylab = PyImport_ImportModule("matplotlib.pylab");

        M_CHECK_PY_PTR_RETURN( pylab, "import matplotlib.pylab failed." );

        Py_DECREF(matplotlib);

        //---------------------------------------------------------------------
        // Grabbing pylab's funciton dictionary

        PyObject * pylab_dict = PyModule_GetDict(pylab);

        M_CHECK_PY_PTR_RETURN( pylab_dict, "dir(pylab) failed" );

        table_.insert(StringPyObjectPair("pylab", pylab_dict));

        Py_DECREF(pylab);

        std::vector< std::string > functions;

        functions.push_back("axhline");
        functions.push_back("axis");
        functions.push_back("axvline");
        functions.push_back("close");
        functions.push_back("detrend");
        functions.push_back("figure");
        functions.push_back("gca");
        functions.push_back("hold");
        functions.push_back("imshow");
        functions.push_back("legend");
        functions.push_back("plot");
        functions.push_back("show");
        functions.push_back("subplot");
        functions.push_back("text");
        functions.push_back("title");
        functions.push_back("window_hanning");
        functions.push_back("xlabel");
        functions.push_back("xlim");
        functions.push_back("ylabel");
        functions.push_back("ylim");

        const uint32 N_FUNCTIONS = static_cast<uint32>(functions.size());

        for(uint32 i = 0; i < N_FUNCTIONS; ++i)
        {
            PyObject * func = PyDict_GetItemString(
            	pylab_dict, functions[i].c_str());

            M_CHECK_PY_PTR_RETURN(
                func, "PyDict_GetItemString('" << functions[i] << "') failed");

            if(! PyCallable_Check(func))
            {
                M_THROW("pylab." << functions[i] << " isn't callable?!");
            }

            table_.insert(
                StringPyObjectPair("pylab." + functions[i], func));
        }

        // Globals

        PyObject * __main__ = PyImport_ImportModule("__main__");

        M_CHECK_PY_PTR_RETURN(__main__, "PyImport_ImportModule() failed");

        PyObject * globals = PyModule_GetDict(__main__);

        M_CHECK_PY_PTR_RETURN(globals, "globals() failed");

        table_.insert(StringPyObjectPair("globals", globals));

        Py_DECREF(__main__);

        run_string("import matplotlib.pylab as _pylab_nsound_");

        Plotter::state_ = INITALIZED;

    #endif
}

//-----------------------------------------------------------------------------
Plotter::
~Plotter()
{
    #ifdef NSOUND_C_PYLAB

        --count_;

        if(count_ == 0)
        {
            #ifndef NSOUND_IN_PYTHON_MODULE

                Py_Finalize();

            #endif

            Plotter::state_ = UNLOADED;
        }

    #endif
}

void
Plotter::
show()
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        // Call pylab.show()

        PyObject * ret = PyObject_CallObject(table_["pylab.show"], NULL);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.show() failed");

        Py_DECREF(ret);

    #else
        // This function is overwridden in the Python module,
        // See the file shadowed.i for how this is implemented in Python module

        #ifndef NSOUND_IN_PYTHON_MODULE

            static bool once = true;

            if(once)
            {
                once = false;
                std::cerr
                    << "WARNING: Nsound::Plotter::show(): Nsound wasn't "
                    "compiled with matplotlib" << std::endl;
                std::cerr.flush();
            }
        #endif
    #endif
}

//-----------------------------------------------------------------------------
PyObject *
Plotter::
_make_kwargs(const std::string & kwargs) const
{
    #ifdef NSOUND_C_PYLAB

        if(kwargs == "") return PyDict_New();

        std::stringstream ss;

        ss << "dict(" << kwargs << ")\n";

        PyCodeObject * code = (PyCodeObject*) Py_CompileString(
            ss.str().c_str(), "Plotter.cc", Py_eval_input);

        M_CHECK_PY_PTR(code, "Py_CompileString() failed");

        PyObject * globals = table_["globals"];

        PyObject * locals = PyDict_New();

        M_CHECK_PY_PTR(locals, "PyDict_New() failed");

        PyObject * kwdict = PyEval_EvalCode(code, globals, locals);

        Py_DECREF(code);
        Py_DECREF(locals);

		return kwdict;

    #endif

    return NULL;
}

void
Plotter::
axhline(
    const float64 & y_pos,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(d)", y_pos);

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.axhline"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.axhline() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
axvline(
    const float64 & x_pos,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(d)", x_pos);

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.axvline"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.axvline() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
xlim(
    const float64 & xmin,
    const float64 & xmax)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        M_ASSERT_VALUE(xmin, <, xmax);

        PyObject * args = Py_BuildValue("(d, d)", xmin, xmax);

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = NULL; //_make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.xlim"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.xlim() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
ylim(
    const float64 & ymin,
    const float64 & ymax)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        M_ASSERT_VALUE(ymin, <, ymax);

        PyObject * args = Py_BuildValue("(d, d)", ymin, ymax);

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = NULL; //_make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.ylim"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.ylim() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
figure(const std::string & kwargs) const
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

//~        if(Plotter::state_ != INITALIZED || hold_is_on_)
//~        {
//~            hold_is_on_ = false; // toggle it.
//~            return;
//~        }

        PyObject * args = Py_BuildValue("()");

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.figure"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.figure() failed");

        Py_DECREF(ret);

//~        // Reset limits
//~        xmax_ = -1e20;
//~        xmin_ = 1e20;
//~        ymin_ = xmin_;
//~        ymax_ = xmax_;

    #endif
}

void
Plotter::
grid(boolean flag)
{
//~    grid_is_on_ = flag;

        if(flag)
        {
            run_string(
                "try:\n"
                "    ax__ = _pylab_nsound_.gca();\n"
                "    ax__.grid(True);\n"
                "except:\n"
                "    pass\n\n");
        }

        else
        {
            run_string(
                "try:\n"
                "    ax__ = _pylab_nsound_.gca();\n"
                "    ax__.grid(False);\n"
                "except:\n"
                "    pass\n\n");
        }


}

void
Plotter::
hide()
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(s)", "all");

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        PyObject * kw_args = NULL; //_make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.close"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.close() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
hold(boolean flag)
{
    #ifdef NSOUND_C_PYLAB

        hold_is_on_ = flag;

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        // Put the list in a tuple.
        PyObject * args = NULL;

        if(hold_is_on_)
        {
            args = PyTuple_Pack(1, Py_True);
        }
        else
        {
            args = PyTuple_Pack(1, Py_False);
        }

        PyObject * ret = PyObject_CallObject(table_["pylab.hold"], args);

        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.hold() failed");

        if(hold_is_on_)
        {
            grid(false);
        }

    #endif
}

void
Plotter::
imagesc(const AudioStream & Z, const std::string & kwargs)
{
	imagesc(Buffer(), Buffer(), Z, kwargs);
}

void
Plotter::
imagesc(
	const Buffer & x_axis,
	const Buffer & y_axis,
	const AudioStream & Z,
	const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        uint32 M = Z.getNChannels();
        uint32 N = Z.getLength();
        uint32 x_size = x_axis.getLength();
        uint32 y_size = y_axis.getLength();

        if(N == 0)
        {
            M_THROW("Nsound::Plotter::imagesc(): "
                 << "Z has zero columns! ("
                 << M
                 << " x "
                 << N
                 << ")");
            return;
        }

        if(x_size > 0 && x_size != N)
        {
            M_THROW("Nsound::Plotter::imagesc(): "
                 << "shape mismatch between x_axis and Z ("
                 << x_size
                 << " != "
                 << N
                 << ")");
            return;
        }

        if(y_size > 0 && y_size != M)
        {
            M_THROW("Nsound::Plotter::imagesc(): "
                 << "shape mismatch between y_axis and Z ("
                 << y_size
                 << " != "
                 << M
                 << ")");
            return;
        }

        float64 xmin = 0.0;
        float64 xmax = 0.0;
        float64 ymin = 0.0;
        float64 ymax = 0.0;

        if(x_size == 0 && y_size == 0)
        {
            xmin = -0.5;
            xmax = static_cast<float64>(N - 1) + 0.5;
            ymin = -0.5;
            ymax = static_cast<float64>(M - 1) + 0.5;
        }
        else if(x_size > 0 && y_size > 0)
        {
            float64 dx = x_axis[1] - x_axis[0];
            float64 dy = y_axis[1] - y_axis[0];

            xmin = x_axis[0]     - 0.5 * dx;
            xmax = x_axis[N - 1] + 0.5 * dx;
            ymin = y_axis[0]     - 0.5 * dy;
            ymax = y_axis[M - 1] + 0.5 * dy;
        }

        // Create python list of lists to represent the matrix

        PyObject * matrix_list = PyList_New(M);

        M_CHECK_PY_PTR_RETURN(matrix_list, "PyList_New() failed");

        for(uint32 m = 0; m < M; ++m)
        {
            PyObject * array = makePyListFromBuffer(Z[m]);

            M_CHECK_PY_PTR_RETURN(array, "makePyListFromBuffer() failed");

            PyList_SET_ITEM(matrix_list, m, array);
        }

        // Now build imshow's arguments and keyword args

        PyObject * args = Py_BuildValue("(O)", matrix_list);

        M_CHECK_PY_PTR_RETURN(args, "Py_BuildValue() failed");

        // build keywords
        std::stringstream ss;

        ss << "extent=["
            << xmin << "," << xmax << "," << ymin << "," << ymax << "],"
            << "interpolation='nearest',"
            << "origin='lower'";

        if(kwargs != "")
        {
            ss << "," << kwargs;
        }

        PyObject * kw_args = _make_kwargs(ss.str());

        // Call pylab.plot().
        PyObject * ret = PyObject_Call(table_["pylab.imshow"], args, kw_args);

        Py_DECREF(args);
        Py_XDECREF(kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.imshow() failed");

        Py_DECREF(ret);

        ///////////////////////////////////////////////////////////////////////
        // Axis tight & Grid

        run_string(
            "try:\n"
            "    ax__ = _pylab_nsound_.gca();\n"
            "    trash__ = ax__.axis('tight');\n"
            "except:\n"
            "    pass\n\n");

        if(grid_is_on_)
        {
            run_string(
                "try:\n"
                "    ax__ = _pylab_nsound_.gca();\n"
                "    ax__.grid(True);\n"
                "except:\n"
                "    pass\n\n");
        }

    #endif
}

void
Plotter::
legend(const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        // Create args & kwargs.
        PyObject * args = Py_BuildValue("()");
        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.legend"], args, kw_args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.legend() failed");

        Py_DECREF(args);
        Py_DECREF(kw_args);

    #endif
}

void
Plotter::
plot(
    const Buffer & buffer,
    const std::string & fmt,
    const std::string & kwargs)
{
    plot(Buffer(), buffer, fmt, kwargs);
}

void
Plotter::
plot(
    const Buffer & x,
    const Buffer & y,
    const std::string & fmt,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        M_ASSERT_VALUE(y.getLength(), >, 0);

        PyObject * x_list = NULL;

        // X may be empty here.
        if(x.getLength() > 0)
        {
            M_ASSERT_VALUE(x.getLength(), ==, y.getLength());

            x_list = makePyListFromBuffer(x);
        }

        PyObject * y_list = makePyListFromBuffer(y);

        // Put the list in a tuple.
        PyObject * args = NULL;

        // Count how many items are going into args.
        uint32 n_args = 1;

        if(x_list != NULL) ++n_args;
        if(fmt != "") ++n_args;

        args = PyTuple_New(n_args);

        M_CHECK_PY_PTR_RETURN(args, "PyTuple_New() failed");

        Py_ssize_t pos = 0;

        if(x_list != NULL)
        {
            PyTuple_SetItem(args, pos, x_list);
            pos += 1;
        }

        PyTuple_SetItem(args, pos, y_list);
        pos += 1;

        PyObject * fmt_str = NULL;

        if(fmt != "")
        {
            fmt_str = PyString_FromString(fmt.c_str());
            M_CHECK_PY_PTR_RETURN(
                fmt_str, "PyString_FromString('" << fmt << "') failed");
            PyTuple_SetItem(args, pos, fmt_str);
        }

        PyObject * kw_args = _make_kwargs(kwargs);

        // Call pylab.plot().
        PyObject * ret = PyObject_Call(table_["pylab.plot"], args, kw_args);

        Py_DECREF(kw_args);
        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.plot() failed");

        Py_DECREF(ret);

//~        ///////////////////////////////////////////////////////////////////////
//~        // Setup plot limits to make stuff fit nicely

//~        float64 xmin = 0.0;
//~        float64 xmax = y.getLength() - 1;

//~        if(x.getLength() > 0)
//~        {
//~            xmin = x[0];
//~            xmax = x[x.getLength() - 1];
//~        }

//~        float64 ymin = y.getMin();
//~        float64 ymax = y.getMax();

//~        float64 margin = 0.05;

//~        float64 dist = xmax - xmin;

//~        xmin -= margin * dist;
//~        xmax += margin * dist;

//~        dist = ymax - ymin;

//~        ymin -= margin * dist;
//~        ymax += margin * dist;

//~        // Create python list for [xmin, xmax, ymin, ymax]

//~        PyObject * limits = PyList_New(4);
//~        PyList_SET_ITEM(limits, 0, PyFloat_FromDouble(xmin - 1e-19));
//~        PyList_SET_ITEM(limits, 1, PyFloat_FromDouble(xmax + 1e-19));
//~        PyList_SET_ITEM(limits, 2, PyFloat_FromDouble(ymin - 1e-19));
//~        PyList_SET_ITEM(limits, 3, PyFloat_FromDouble(ymax + 1e-19));

//~        args = PyTuple_Pack(1, limits);

//~        // Call pylab.axis().
//~        ret = PyObject_CallObject(table_["pylab.axis"], args);

//~        Py_DECREF(args);
//~        Py_DECREF(limits);

//~        if(!ret)
//~        {
//~            PyErr_Print();
//~            cerr << PRINT_LINE
//~                 << "failed to call python pylab.axis()"
//~                 << "\n";
//~            return;
//~        }

        if(grid_is_on_)
        {
            run_string(
                "try:\n"
                "    ax__ = _pylab_nsound_.gca();\n"
                "    ax__.grid(True);\n"
                "except:\n"
                "    pass\n\n");
        }

    #endif
}

void
Plotter::
plot(
    const float64 & x,
    const float64 & y,
    const std::string & fmt,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

       Buffer xx(1); xx << x;
       Buffer yy(1); yy << y;

       plot(xx, yy, fmt, kwargs);

    #endif
}

void
Plotter::
run_string(const std::string & command) const
{
    #ifdef NSOUND_C_PYLAB

        std::string str = command + "\n";

        PyObject * ret = PyRun_String(
            str.c_str(), Py_file_input, table_["globals"], NULL);

        M_CHECK_PY_PTR_RETURN(ret, "run_string() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
savefig(const std::string & filename, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        std::stringstream ss;

        ss << "savefig('"
           << filename
           << "'";

        if(kwargs.length() > 0)
        {
            ss << ", " << kwargs;
        }

        ss << ")";

        run_string(ss.str());

//~        std::string command(ss.str());

//~        PyObject * ret = PyRun_String(command.c_str(), Py_eval_input, table_["pylab"], NULL);

//~        if(!ret)
//~        {
//~            PyErr_Print();
//~            cerr << PRINT_LINE
//~                 << "failed to call python '"
//~                 << command
//~                 << "'"
//~                 << "\n";
//~            return;
//~        }

    #endif
}

void
Plotter::
set_xscale(const std::string & s)
{
    if(s != "log" && s != "linear") return;

    std::stringstream ss;

    ss
        << "try:\n"
        << "    ax = _pylab_nsound_.gca();\n"
        << "    ax.set_xscale('" << s << "')\n"
        << "except:\n"
        << "    pass\n\n";

    run_string(ss.str());
}

void
Plotter::
set_yscale(const std::string & s)
{
    if(s != "log" && s != "linear") return;

    std::stringstream ss;

    ss
        << "try:\n"
        << "    ax = _pylab_nsound_.gca();\n"
        << "    ax.set_yscale('" << s << "')\n"
        << "except:\n"
        << "    pass\n\n";

    run_string(ss.str());
}

Axes
Plotter::
subplot(
    const uint32 n_rows,
    const uint32 n_columns,
    const uint32 n,
    const std::string & kwargs,
    Axes * sharex,
    Axes * sharey)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return Axes(NULL);
        }

        // args
        PyObject * args = Py_BuildValue("(iii)", n_rows, n_columns, n);

        // kwargs
        PyObject * kw_args = _make_kwargs(kwargs);

        // Stick in sharex & sharey
        if(sharex != NULL)
        {
            PyObject * key = PyString_FromString("sharex");
            PyDict_SetItem(kw_args, key, sharex->get_axes());
        }

        if(sharey != NULL)
        {
            PyObject * key = PyString_FromString("sharey");
            PyDict_SetItem(kw_args, key, sharey->get_axes());
        }

        // Call pylab.subplot()
        PyObject * ret = PyObject_Call(table_["pylab.subplot"], args, kw_args);

        Py_DECREF(args);
        Py_DECREF(kw_args);

        M_CHECK_PY_PTR(ret, "pylab.subplot() failed");

        if(grid_is_on_)
        {
            run_string(
                "try:\n"
                "    ax = _pylab_nsound_.gca();\n"
                "    ax.grid(True);\n"
                "except:\n"
                "    pass\n\n");
        }

        return Axes(ret);

    #endif

    return Axes(NULL);
}

void
Plotter::
text(
    const float64 & x,
    const float64 & y,
    const std::string & text,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        // Create a python string.

        PyObject * args = Py_BuildValue("(dds)",
            x, y, text.c_str());

        PyObject * kw_args = _make_kwargs(kwargs);

        // Call pylab.title()
        PyObject * ret = PyObject_Call(table_["pylab.text"], args, kw_args);

        Py_DECREF(kw_args);
        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.text() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
title(const std::string & title, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(s)", title.c_str());
        PyObject * kw_args = _make_kwargs(kwargs);

        // Call pylab.title()
        PyObject * ret = PyObject_Call(table_["pylab.title"], args, kw_args);

        Py_DECREF(kw_args);
        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.title() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
xlabel(const std::string & label, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(s)", label.c_str());
        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.xlabel"], args, kw_args);

        Py_DECREF(kw_args);
        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.xlabel() failed");

        Py_DECREF(ret);

    #endif
}

void
Plotter::
ylabel(const std::string & label, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB

        if(Plotter::state_ != INITALIZED)
        {
            return;
        }

        PyObject * args = Py_BuildValue("(s)", label.c_str());
        PyObject * kw_args = _make_kwargs(kwargs);

        PyObject * ret = PyObject_Call(table_["pylab.ylabel"], args, kw_args);

        Py_DECREF(kw_args);
        Py_DECREF(args);

        M_CHECK_PY_PTR_RETURN(ret, "pylab.ylabel() failed");

        Py_DECREF(ret);

    #endif
}

#ifdef NSOUND_C_PYLAB
    PyObject *
    Plotter::
    makePyListFromBuffer(const Buffer & buffer) const
    {
        uint32 n_samples = buffer.getLength();

        PyObject * py_list = PyList_New(n_samples);

        M_CHECK_PY_PTR(py_list, "PyList_New() failed");

        for(uint32 i = 0; i < n_samples; ++i)
        {
            PyList_SET_ITEM(py_list, i,
                PyFloat_FromDouble(static_cast<float64>(buffer[i])));
        }

        return py_list;
    }

    PyObject *
    Plotter::
    makePyIntFromUint32(const uint32 & i) const
    {
        PyObject * py_int = PyInt_FromLong(i);

        M_CHECK_PY_PTR(py_int, "PyInt_FromLong() failed");

        return py_int;
    }

#endif

// :mode=c++:
