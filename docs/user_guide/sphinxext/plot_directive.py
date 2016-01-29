"""A special directive for including a matplotlib plot.

The source code for the plot may be included in one of two ways:

  1. A path to a source file as the argument to the directive::

       .. plot:: path/to/plot.py

     When a path to a source file is given, the content of the
     directive may optionally contain a caption for the plot::

       .. plot:: path/to/plot.py

          This is the caption for the plot

     Additionally, one my specify the name of a function to call (with
     no arguments) immediately after importing the module::

       .. plot:: path/to/plot.py plot_function1

  2. Included as inline content to the directive::

     .. plot::

        import matplotlib.pyplot as plt
        import matplotlib.image as mpimg
        import numpy as np
        img = mpimg.imread('_static/stinkbug.png')
        imgplot = plt.imshow(img)

In HTML output, `plot` will include a .png file with a link to a high-res
.png and .pdf.  In LaTeX output, it will include a .pdf.

To customize the size of the plot, this directive supports all of the
options of the `image` directive, except for `target` (since plot will
add its own target).  These include `alt`, `height`, `width`, `scale`,
`align` and `class`.

Additionally, if the `:include-source:` option is provided, the
literal source will be displayed inline in the text, (as well as a
link to the source in HTML).  If this source file is in a non-UTF8 or
non-ASCII encoding, the encoding must be specified using the
`:encoding:` option.

The set of file formats to generate can be specified with the
`plot_formats` configuration variable.


Error handling:

Any errors generated during the running of the code are emitted as warnings
using the Python `warnings` module, using a custom category called
`PlotWarning`.  To turn the warnings into fatal errors that stop the
documentation build, after adjusting your `sys.path` in your `conf.py` Sphinx
configuration file, use::

    import plot_directive
    warnings.simplefilter('error', plot_directive.PlotWarning)
"""

import sys, os, shutil, imp, warnings, cStringIO, re
try:
    from hashlib import md5
except ImportError:
    from md5 import md5

from docutils.parsers.rst import directives
try:
    # docutils 0.4
    from docutils.parsers.rst.directives.images import align
except ImportError:
    # docutils 0.5
    from docutils.parsers.rst.directives.images import Image
    align = Image.align
import sphinx

sphinx_version = sphinx.__version__.split(".")
# The split is necessary for sphinx beta versions where the string is
# '6b1'
sphinx_version = tuple([int(re.split('[a-z]', x)[0])
                        for x in sphinx_version[:2]])

import matplotlib
matplotlib.use('Agg')
import matplotlib.cbook as cbook
import matplotlib.pyplot as plt
import matplotlib.image as image
from matplotlib import _pylab_helpers
from matplotlib.sphinxext import only_directives


class PlotWarning(Warning):
    """Warning category for all warnings generated by this directive.

    By printing our warnings with this category, it becomes possible to turn
    them into errors by using in your conf.py::

      warnings.simplefilter('error', plot_directive.PlotWarning)

    This way, you can ensure that your docs only build if all your examples
    actually run successfully.
    """
    pass


# os.path.relpath is new in Python 2.6
if hasattr(os.path, 'relpath'):
    relpath = os.path.relpath
else:
    # This code is snagged from Python 2.6

    def relpath(target, base=os.curdir):
        """
        Return a relative path to the target from either the current dir or an optional base dir.
        Base can be a directory specified either as absolute or relative to current dir.
        """

        if not os.path.exists(target):
            raise OSError, 'Target does not exist: '+target

        if not os.path.isdir(base):
            raise OSError, 'Base is not a directory or does not exist: '+base

        base_list = (os.path.abspath(base)).split(os.sep)
        target_list = (os.path.abspath(target)).split(os.sep)

        # On the windows platform the target may be on a completely
        # different drive from the base.
        if os.name in ['nt','dos','os2'] and base_list[0] <> target_list[0]:
            raise OSError, 'Target is on a different drive to base. Target: '+target_list[0].upper()+', base: '+base_list[0].upper()

        # Starting from the filepath root, work out how much of the
        # filepath is shared by base and target.
        for i in range(min(len(base_list), len(target_list))):
            if base_list[i] <> target_list[i]: break
        else:
            # If we broke out of the loop, i is pointing to the first
            # differing path elements.  If we didn't break out of the
            # loop, i is pointing to identical path elements.
            # Increment i so that in all cases it points to the first
            # differing path elements.
            i+=1

        rel_list = [os.pardir] * (len(base_list)-i) + target_list[i:]
        if rel_list:
            return os.path.join(*rel_list)
        else:
            return ""

template = """
.. htmlonly::

   %(links)s

   .. figure:: %(prefix)s%(tmpdir)s/%(outname)s.png
%(options)s

%(caption)s

.. // Nick changed the below format to use image, this will force the image
.. // to appear inline with the text.

.. latexonly::
    .. image:: %(prefix)s%(tmpdir)s/%(outname)s.pdf
        :scale: 75 %%
        :align: center

%(caption)s

"""

exception_template = """
.. htmlonly::

   [`source code <%(linkdir)s/%(basename)s.py>`__]

Exception occurred rendering plot.

"""

template_content_indent = '      '

def out_of_date(original, derived):
    """
    Returns True if derivative is out-of-date wrt original,
    both of which are full file paths.
    """
    return (not os.path.exists(derived) or
            (os.path.exists(original) and
             os.stat(derived).st_mtime < os.stat(original).st_mtime))

def run_code(plot_path, function_name, plot_code):
    """
    Import a Python module from a path, and run the function given by
    name, if function_name is not None.
    """
    # Change the working directory to the directory of the example, so
    # it can get at its data files, if any.  Add its path to sys.path
    # so it can import any helper modules sitting beside it.
    if plot_code is not None:
        exec_code = 'import numpy as np; import matplotlib.pyplot as plt\n%s'%plot_code

        try:
            exec(exec_code)
        except Exception as e:
            message =(
                "FAILED TO EXECUTE:\n" +
                exec_code +
                "\n\nException: %s" % str(e))

            sys.stderr.write(message)

            raise Exception(message)
    else:
        pwd = os.getcwd()
        path, fname = os.path.split(plot_path)
        sys.path.insert(0, os.path.abspath(path))
        stdout = sys.stdout
        sys.stdout = cStringIO.StringIO()
        os.chdir(path)
        fd = None
        try:
            fd = open(fname)
            module = imp.load_module(
                "__plot__", fd, fname, ('py', 'r', imp.PY_SOURCE))
        finally:
            del sys.path[0]
            os.chdir(pwd)
            sys.stdout = stdout
            if fd is not None:
                fd.close()

        if function_name is not None:
            getattr(module, function_name)()

def run_savefig(plot_path, basename, tmpdir, destdir, formats):
    """
    Once a plot script has been imported, this function runs savefig
    on all of the figures in all of the desired formats.
    """
    fig_managers = _pylab_helpers.Gcf.get_all_fig_managers()
    for i, figman in enumerate(fig_managers):
        for j, (format, dpi) in enumerate(formats):
            if len(fig_managers) == 1:
                outname = basename
            else:
                outname = "%s_%02d" % (basename, i)
            outname = outname + "." + format
            outpath = os.path.join(tmpdir, outname)
            try:
                figman.canvas.figure.savefig(outpath, dpi=dpi)
            except:
                s = cbook.exception_to_str("Exception saving plot %s" % plot_path)
                warnings.warn(s, PlotWarning)
                return 0
            if j > 0:
                shutil.copyfile(outpath, os.path.join(destdir, outname))

    return len(fig_managers)

def clear_state():
    plt.close('all')
    matplotlib.rcdefaults()
    # Set a default figure size that doesn't overflow typical browser
    # windows.  The script is free to override it if necessary.
    matplotlib.rcParams['figure.figsize'] = (5.5, 4.5)

def render_figures(plot_path, function_name, plot_code, tmpdir, destdir,
                   formats):
    """
    Run a pyplot script and save the low and high res PNGs and a PDF
    in outdir.
    """
    plot_path = str(plot_path)  # todo, why is unicode breaking this
    basedir, fname = os.path.split(plot_path)
    basename, ext = os.path.splitext(fname)

    all_exists = True

    # Look for single-figure output files first
    for format, dpi in formats:
        outname = os.path.join(tmpdir, '%s.%s' % (basename, format))
        if out_of_date(plot_path, outname):
            all_exists = False
            break

    if all_exists:
        return 1

    # Then look for multi-figure output files, assuming
    # if we have some we have all...
    i = 0
    while True:
        all_exists = True
        for format, dpi in formats:
            outname = os.path.join(
                tmpdir, '%s_%02d.%s' % (basename, i, format))
            if out_of_date(plot_path, outname):
                all_exists = False
                break
        if all_exists:
            i += 1
        else:
            break

    if i != 0:
        return i

    # We didn't find the files, so build them

    clear_state()
    try:
        run_code(plot_path, function_name, plot_code)
    except Exception as e:

        message = (
            "Exception occured while executing plot code:\n" +
            plot_code +
            "\n\n" + str(e))

        raise Exception(message)

    num_figs = run_savefig(plot_path, basename, tmpdir, destdir, formats)

    if '__plot__' in sys.modules:
        del sys.modules['__plot__']

    return num_figs

def _plot_directive(plot_path, basedir, function_name, plot_code, caption,
                    options, state_machine):
    formats = setup.config.plot_formats
    if type(formats) == str:
        formats = eval(formats)

    fname = os.path.basename(plot_path)
    basename, ext = os.path.splitext(fname)

    # Get the directory of the rst file, and determine the relative
    # path from the resulting html file to the plot_directive links
    # (linkdir).  This relative path is used for html links *only*,
    # and not the embedded image.  That is given an absolute path to
    # the temporary directory, and then sphinx moves the file to
    # build/html/_images for us later.
    rstdir, rstfile = os.path.split(state_machine.document.attributes['source'])
    outdir = os.path.join('plot_directive', basedir)
    reldir = relpath(setup.confdir, rstdir)
    linkdir = os.path.join(reldir, outdir)

    # tmpdir is where we build all the output files.  This way the
    # plots won't have to be redone when generating latex after html.

    # Prior to Sphinx 0.6, absolute image paths were treated as
    # relative to the root of the filesystem.  0.6 and after, they are
    # treated as relative to the root of the documentation tree.  We
    # need to support both methods here.
    tmpdir = os.path.join('build', outdir)
    tmpdir = os.path.abspath(tmpdir)

    if sphinx_version < (0, 6):
        prefix = ''
    else:
        prefix = '/'

    if not os.path.exists(tmpdir):
        cbook.mkdirs(tmpdir)

    # destdir is the directory within the output to store files
    # that we'll be linking to -- not the embedded images.
    destdir = os.path.abspath(os.path.join(setup.app.builder.outdir, outdir))
    if not os.path.exists(destdir):
        cbook.mkdirs(destdir)

    # Properly indent the caption
    caption = '\n'.join(template_content_indent + line.strip()
                        for line in caption.split('\n'))

    # Generate the figures, and return the number of them
    num_figs = render_figures(plot_path, function_name, plot_code, tmpdir,
                              destdir, formats)

    # Now start generating the lines of output
    lines = []

    if plot_code is None:
        shutil.copyfile(plot_path, os.path.join(destdir, fname))

    if options.has_key('include-source'):
        if plot_code is None:
            if sphinx_version > (1,):
                include_prefix = '/'
            else:
                include_prefix = setup.app.builder.srcdir

            lines.extend(
                ['.. include:: %s' % os.path.join(include_prefix, plot_path),
                 '    :literal:'])
            if options.has_key('encoding'):
                lines.append('    :encoding: %s' % options['encoding'])
                del options['encoding']
        else:
            lines.extend(['::', ''])
            lines.extend(['    %s' % row.rstrip()
                          for row in plot_code.split('\n')])
        lines.append('')
        del options['include-source']
    else:
        lines = []

    if num_figs > 0:
        options = ['%s:%s: %s' % (template_content_indent, key, val)
                   for key, val in options.items()]
        options = "\n".join(options)

        for i in range(num_figs):
            if num_figs == 1:
                outname = basename
            else:
                outname = "%s_%02d" % (basename, i)

            # Copy the linked-to files to the destination within the build tree,
            # and add a link for them
            links = []
            if plot_code is None:
                links.append('`source code <%(linkdir)s/%(basename)s.py>`__')
            for format, dpi in formats[1:]:
                links.append('`%s <%s/%s.%s>`__' % (format, linkdir, outname, format))
            if len(links):
                links = '[%s]' % (', '.join(links) % locals())
            else:
                links = ''

            lines.extend((template % locals()).split('\n'))
    else:
        lines.extend((exception_template % locals()).split('\n'))

    if len(lines):
        state_machine.insert_input(
            lines, state_machine.input_lines.source(0))

#~    print "DEBUG START"
#~    for l in lines:
#~        print l
#~    print "DEBUG STOP"

    return []

def plot_directive(name, arguments, options, content, lineno,
                   content_offset, block_text, state, state_machine):
    """
    Handle the arguments to the plot directive.  The real work happens
    in _plot_directive.
    """
    # The user may provide a filename *or* Python code content, but not both
    if len(arguments):
        plot_path = directives.uri(arguments[0])
        basedir = relpath(os.path.dirname(plot_path), setup.app.builder.srcdir)

        # If there is content, it will be passed as a caption.

        # Indent to match expansion below.  XXX - The number of spaces matches
        # that of the 'options' expansion further down.  This should be moved
        # to common code to prevent them from diverging accidentally.
        caption = '\n'.join(content)

        # If the optional function name is provided, use it
        if len(arguments) == 2:
            function_name = arguments[1]
        else:
            function_name = None

        return _plot_directive(plot_path, basedir, function_name, None, caption,
                               options, state_machine)
    else:
        plot_code = '\n'.join(content)

        # Since we don't have a filename, use a hash based on the content
        plot_path = md5(plot_code).hexdigest()[-10:]

        return _plot_directive(plot_path, 'inline', None, plot_code, '', options,
                               state_machine)

def mark_plot_labels(app, document):
    """
    To make plots referenceable, we need to move the reference from
    the "htmlonly" (or "latexonly") node to the actual figure node
    itself.
    """
    for name, explicit in document.nametypes.iteritems():
        if not explicit:
            continue
        labelid = document.nameids[name]
        if labelid is None:
            continue
        node = document.ids[labelid]
        if node.tagname in ('html_only', 'latex_only'):
            for n in node:
                if n.tagname == 'figure':
                    sectname = name
                    for c in n:
                        if c.tagname == 'caption':
                            sectname = c.astext()
                            break

                    node['ids'].remove(labelid)
                    node['names'].remove(name)
                    n['ids'].append(labelid)
                    n['names'].append(name)
                    document.settings.env.labels[name] = \
                        document.settings.env.docname, labelid, sectname
                    break

def setup(app):
    setup.app = app
    setup.config = app.config
    setup.confdir = app.confdir

    options = {'alt': directives.unchanged,
               'height': directives.length_or_unitless,
               'width': directives.length_or_percentage_or_unitless,
               'scale': directives.nonnegative_int,
               'align': align,
               'class': directives.class_option,
               'include-source': directives.flag,
               'encoding': directives.encoding }

    app.add_directive('plot', plot_directive, True, (0, 2, 0), **options)
    app.add_config_value(
        'plot_formats',
        [('png', 80), ('hires.png', 200), ('pdf', 50)],
        True)

    app.connect('doctree-read', mark_plot_labels)