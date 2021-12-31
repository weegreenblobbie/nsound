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

        print("I love Python, the current time is %s"''' % datetime.now())

"""
import sys

from docutils.parsers.rst import Directive

class PyExec(Directive):

    has_content = True
    required_arguments = 0
    optional_arguments = 0

    def run(self):

        py_code = "\n".join(self.content)

        globals_ = globals()
        locals_ = locals()

        try:
            exec(py_code, globals_, locals_)
            template = locals_['template']

        except Exception as e:
            message = f"""

ERROR: An exception occured during the evaluation of the pyexec directive

The offending code was:

{py_code}

"""
            sys.stderr.write(message)
            sys.stderr.flush()
            raise

        lines = ["%s" % row.rstrip() for row in template.split('\n')]
        self.state_machine.insert_input(
            lines, self.state_machine.input_lines.source(0))

        return []


def setup(app):
    app.add_directive('pyexec', PyExec, True)

