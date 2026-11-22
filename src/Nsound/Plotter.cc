//-----------------------------------------------------------------------------
//
//  $Id: Plotter.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/Nsound.h>

#ifdef NSOUND_C_PYLAB
    #include <pybind11/embed.h>
#endif

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/Plotter.h>

#if defined(NSOUND_C_PYLAB) && ! defined(NSOUND_IN_PYTHON_MODULE)
    #include "rules_cc/cc/runfiles/runfiles.h"

    #if defined(NSOUND_PLATFORM_OS_WINDOWS)
        // No min() and max() macros, Bazel's MSVC toolchain may say so too.
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <windows.h>
    #elif defined(NSOUND_PLATFORM_OS_MAC)
        #include <mach-o/dyld.h>
    #else
        #include <unistd.h>
    #endif
#endif

#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Nsound;

#ifdef NSOUND_C_PYLAB

namespace py = pybind11;

#ifndef NSOUND_IN_PYTHON_MODULE

namespace Nsound
{

// The runfiles paths of Bazel's Python, from //bazel:embedded_python.bzl.
extern const char * const EMBEDDED_PYTHON_INTERPRETER;
extern const char * const EMBEDDED_PYTHON_HOME;
extern const char * const EMBEDDED_PYTHON_PATHS[];

};

#endif

namespace
{

#ifndef NSOUND_IN_PYTHON_MODULE

//! Returns this program's path, where the runfiles library finds the
//! runfiles when Bazel didn't say where they are.
std::string
executable_path()
{
    #if defined(NSOUND_PLATFORM_OS_WINDOWS)

        // Long enough for a long path.
        std::vector<char> path(32768);
        DWORD n = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
        return std::string(path.data(), n);

    #elif defined(NSOUND_PLATFORM_OS_MAC)

        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);

        std::vector<char> path(size);
        if(_NSGetExecutablePath(path.data(), &size) != 0) return "";
        return std::string(path.data());

    #else

        std::vector<char> path(4096);
        ssize_t n = readlink("/proc/self/exe", path.data(), path.size());
        return n > 0 ? std::string(path.data(), static_cast<std::size_t>(n)) : "";

    #endif
}

using rules_cc::cc::runfiles::Runfiles;

//! Returns the folder holding the repositories the interpreter's runfile is
//! in, or an empty path when it can't tell.
//
//! When the runfiles library reads the runfiles manifest, Rlocation() of a
//! file is the real file, in Bazel's external folder, otherwise it is the
//! link or copy in the runfiles folder.  Folders aren't in the manifest.
std::filesystem::path
repositories_folder(const Runfiles & runfiles)
{
    namespace fs = std::filesystem;

    fs::path interpreter =
        fs::path(runfiles.Rlocation(EMBEDDED_PYTHON_INTERPRETER)).lexically_normal();

    fs::path relative = fs::path(EMBEDDED_PYTHON_INTERPRETER).lexically_normal();

    fs::path folder = interpreter;

    for(auto part = relative.begin(); part != relative.end(); ++part)
    {
        folder = folder.parent_path();
    }

    if((folder / relative).lexically_normal() != interpreter) return {};

    return folder;
}

//! Returns the path of a runfiles folder in the repositories folder when it
//! is there, otherwise Rlocation() of it.
//
//! Bazel's external folder makes the paths about 100 characters shorter than
//! the runfiles folder: the Windows DLL loader can't find a DLL's DLLs past
//! 260 characters, as for Pillow's _imaging from the runfiles of a program
//! with a long name.
std::string
python_path(
    const Runfiles & runfiles,
    const std::filesystem::path & repositories,
    const char * runfiles_path)
{
    std::error_code error;

    if(! repositories.empty())
    {
        std::filesystem::path real = repositories / runfiles_path;

        if(std::filesystem::exists(real, error))
        {
            return real.make_preferred().string();
        }
    }

    return runfiles.Rlocation(runfiles_path);
}

//! Starts Bazel's Python, with matplotlib on its path.
//
//! Throws std::runtime_error when it can't.
void
start_python()
{
    std::string error;
    std::unique_ptr<Runfiles> runfiles(
        Runfiles::Create(executable_path(), BAZEL_CURRENT_REPOSITORY, &error));

    if(runfiles == nullptr)
    {
        throw std::runtime_error("can't find the program's runfiles: " + error);
    }

    std::filesystem::path repositories = repositories_folder(*runfiles);

    std::string home = python_path(*runfiles, repositories, EMBEDDED_PYTHON_HOME);

    // Isolated from the PYTHON* environment variables and the user's
    // site-packages, it only sees Bazel's packages.
    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);

    // Ctrl-C keeps stopping the program, Python doesn't take SIGINT.
    config.install_signal_handlers = 0;

    // Python runs until the program ends and is never shut down, so it
    // writes its output straight away, nothing waits in a buffer.
    config.buffered_stdio = 0;

    PyStatus status = PyConfig_SetBytesString(&config, &config.home, home.c_str());

    if(PyStatus_Exception(status))
    {
        PyConfig_Clear(&config);
        throw std::runtime_error("can't set the Python home " + home);
    }

    // It frees config.
    py::initialize_interpreter(&config, 0, nullptr, false);

    py::list sys_path = py::module_::import("sys").attr("path");

    for(const char * const * path = EMBEDDED_PYTHON_PATHS; *path != nullptr; ++path)
    {
        sys_path.append(python_path(*runfiles, repositories, *path));
    }
}

#endif

//! Returns matplotlib.pyplot, nullptr when plotting is off.
//
//! The first call imports it, after starting Python in a C++ program.  When
//! that fails it prints why and plotting stays off.  pyplot is never freed:
//! Python may be gone when static objects are destroyed.
py::object *
pyplot()
{
    static py::object * pyplot_ = nullptr;
    static bool tried = false;

    if(tried) return pyplot_;

    tried = true;

    try
    {
        #ifndef NSOUND_IN_PYTHON_MODULE
            if(! Py_IsInitialized()) start_python();
        #endif

        py::object module = py::module_::import("matplotlib.pyplot");

        // run_string() and the kwargs strings can use it by this name.
        py::module_::import("__main__").attr("_pyplot_nsound_") = module;

        pyplot_ = new py::object(module);
    }
    catch(const std::exception & e)
    {
        std::cerr << "WARNING: Nsound::Plotter: plotting is off, " << e.what()
                  << std::endl;
    }

    return pyplot_;
}

//! Returns the Python keyword arguments in kwargs, e.g. "color='red'", as a
//! dict, evaluated in __main__.
py::dict
make_kwargs(const std::string & kwargs)
{
    if(kwargs.empty()) return py::dict();

    py::object globals = py::module_::import("__main__").attr("__dict__");

    return py::eval("dict(" + kwargs + ")", globals);
}

//! Returns the samples as a Python list.
py::list
make_list(const Buffer & buffer)
{
    py::list list;

    for(float64 x : buffer) list.append(x);

    return list;
}

//! Turns on the grid of the current axes, as plot(), imagesc() and
//! subplot() do.
void
grid_on(const py::object & pyplot)
{
    pyplot.attr("gca")().attr("grid")(true);
}

} // namespace

#endif

//-----------------------------------------------------------------------------
Axes::
Axes(PyObject * axes_ptr)
    : axes_ptr_(axes_ptr)
{
    #ifdef NSOUND_C_PYLAB
        Py_XINCREF(axes_ptr_);
    #endif
}

Axes::
Axes(const Axes & copy)
    : axes_ptr_(copy.axes_ptr_)
{
    #ifdef NSOUND_C_PYLAB
        Py_XINCREF(axes_ptr_);
    #endif
}

Axes &
Axes::
operator=(const Axes & rhs)
{
    #ifdef NSOUND_C_PYLAB
        Py_XINCREF(rhs.axes_ptr_);
        Py_XDECREF(axes_ptr_);
    #endif

    axes_ptr_ = rhs.axes_ptr_;

    return *this;
}

Axes::
~Axes()
{
    #ifdef NSOUND_C_PYLAB
        Py_XDECREF(axes_ptr_);
    #endif
}

//-----------------------------------------------------------------------------
Plotter::
Plotter()
{
    #ifdef NSOUND_C_PYLAB
        pyplot();
    #endif
}

void
Plotter::
show()
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("show")();
    #else
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
}

void
Plotter::
axhline(
    const float64 & y_pos,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("axhline")(y_pos, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
axvline(
    const float64 & x_pos,
    const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("axvline")(x_pos, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
xlim(
    const float64 & xmin,
    const float64 & xmax)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        M_ASSERT_VALUE(xmin, <, xmax);

        pyplot_->attr("xlim")(xmin, xmax);
    #endif
}

void
Plotter::
ylim(
    const float64 & ymin,
    const float64 & ymax)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        M_ASSERT_VALUE(ymin, <, ymax);

        pyplot_->attr("ylim")(ymin, ymax);
    #endif
}

void
Plotter::
figure(const std::string & kwargs) const
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("figure")(**make_kwargs(kwargs));
    #endif
}

void
Plotter::
grid(boolean flag) const
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("gca")().attr("grid")(flag);
    #endif
}

void
Plotter::
hide()
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("close")("all");
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
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

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
        }

        if(x_size > 0 && x_size != N)
        {
            M_THROW("Nsound::Plotter::imagesc(): "
                 << "shape mismatch between x_axis and Z ("
                 << x_size
                 << " != "
                 << N
                 << ")");
        }

        if(y_size > 0 && y_size != M)
        {
            M_THROW("Nsound::Plotter::imagesc(): "
                 << "shape mismatch between y_axis and Z ("
                 << y_size
                 << " != "
                 << M
                 << ")");
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

        // A list of rows, one per channel.
        py::list matrix;

        for(uint32 m = 0; m < M; ++m)
        {
            matrix.append(make_list(Z[m]));
        }

        // The defaults, kwargs can change them.
        py::dict kw;
        kw["extent"] = py::make_tuple(xmin, xmax, ymin, ymax);
        kw["interpolation"] = "nearest";
        kw["origin"] = "lower";

        for(auto item : make_kwargs(kwargs))
        {
            kw[item.first] = item.second;
        }

        pyplot_->attr("imshow")(matrix, **kw);

        pyplot_->attr("gca")().attr("axis")("tight");

        grid_on(*pyplot_);
    #endif
}

void
Plotter::
legend(const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("legend")(**make_kwargs(kwargs));
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
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        M_ASSERT_VALUE(y.getLength(), >, 0);

        // pyplot.plot([x,] y [, fmt], **kwargs), x may be empty.
        py::list args;

        if(x.getLength() > 0)
        {
            M_ASSERT_VALUE(x.getLength(), ==, y.getLength());
            args.append(make_list(x));
        }

        args.append(make_list(y));

        if(fmt != "") args.append(fmt);

        pyplot_->attr("plot")(*args, **make_kwargs(kwargs));

        grid_on(*pyplot_);
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
    Buffer xx(1); xx << x;
    Buffer yy(1); yy << y;

    plot(xx, yy, fmt, kwargs);
}

void
Plotter::
run_string(const std::string & command) const
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        py::object globals = py::module_::import("__main__").attr("__dict__");

        py::exec(command, globals);
    #endif
}

void
Plotter::
savefig(const std::string & filename, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("savefig")(filename, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
set_xscale(const std::string & s) const
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        if(s != "log" && s != "linear") return;

        pyplot_->attr("gca")().attr("set_xscale")(s);
    #endif
}

void
Plotter::
set_yscale(const std::string & s) const
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        if(s != "log" && s != "linear") return;

        pyplot_->attr("gca")().attr("set_yscale")(s);
    #endif
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
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return Axes(nullptr);

        py::dict kw = make_kwargs(kwargs);

        if(sharex != nullptr && sharex->get_axes() != nullptr)
        {
            kw["sharex"] = py::reinterpret_borrow<py::object>(sharex->get_axes());
        }

        if(sharey != nullptr && sharey->get_axes() != nullptr)
        {
            kw["sharey"] = py::reinterpret_borrow<py::object>(sharey->get_axes());
        }

        py::object axes = pyplot_->attr("subplot")(n_rows, n_columns, n, **kw);

        grid_on(*pyplot_);

        return Axes(axes.ptr());
    #else
        return Axes(nullptr);
    #endif
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
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("text")(x, y, text, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
title(const std::string & title, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("title")(title, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
xlabel(const std::string & label, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("xlabel")(label, **make_kwargs(kwargs));
    #endif
}

void
Plotter::
ylabel(const std::string & label, const std::string & kwargs)
{
    #ifdef NSOUND_C_PYLAB
        py::object * pyplot_ = pyplot();
        if(pyplot_ == nullptr) return;

        pyplot_->attr("ylabel")(label, **make_kwargs(kwargs));
    #endif
}

// :mode=c++:
