"""
$Id: pyexec_directive.py 589 2010-12-18 11:13:05Z weegreenblobbie $

This Sphinx extentsion allows one to inline python code and have it execute,
the results of the code snippit should be stored in a string with the name
template.

For example, to embed the current date and time, to the following.

.. pyexec::

    from datetime import datetime
    template = "%s" % datetime.now()

Larger strings can be made using Python's tripple quote syntax.

.. pyexec::

    frome datetime import datetime

    template = '''
    This text will get inserted into the document at the pyexec directive
    above.  Inline code can all be used::

        print "I love Python, the current time is %s"''' %(

    datetime.now())

"""

#~import sys, os, shutil, imp, warnings, cStringIO, re
#~try:
#~    from hashlib import md5
#~except ImportError:
#~    from md5 import md5

#~from docutils.parsers.rst import directives

###############################################################################
def pyexec_directive(name, arguments, options, content, lineno,
                   content_offset, block_text, state, state_machine):

    py_code = "\n".join(content)

    try:
        exec(py_code)

    except Exception as e:
        message = ".. warning::\n    An exception occured during the " \
        " evaluation of the pyexec directive\n\nThe offending code was::\n\n"

        for line in py_code.split("\n"):
            message += "    %s\n" % line

        message += "\n\nThe Python exception::\n\n"

        error_message = "%s" % e

        for line in error_message.split("\n"):
            message += "    %s\n" % line

        template = message

    lines = ["%s" % row.rstrip() for row in template.split('\n')]

    state_machine.insert_input(
        lines, state_machine.input_lines.source(0))

    return []

###############################################################################
def setup(app):
    setup.app = app
    setup.config = app.config
    setup.confdir = app.confdir

    options = {}

    app.add_directive('pyexec', pyexec_directive, True, (0, 2, 0), **options)

