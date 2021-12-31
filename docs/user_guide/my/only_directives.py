#
# A pair of directives for inserting content that will only appear in
# either html or latex.
#

from docutils.nodes import Body, Element
from docutils.parsers.rst import directives, Directive

class only_base(Body, Element):
    def dont_traverse(self, *args, **kwargs):
        return []

class html_only(only_base):
    pass

class latex_only(only_base):
    pass

class only_diretive_base(Directive):

    def run(self):
        text = '\n'.join(self.content)
        node = self.node_class(text)
        state.nested_parse(self.content, self.content_offset, node)
        return [node]

class HtmlOnlyDirective(only_diretive_base):
    pass

class LatexOnlyDirective(only_diretive_base):
    pass


def builder_inited(app):
    if app.builder.name == 'html':
        latex_only.traverse = only_base.dont_traverse
    else:
        html_only.traverse = only_base.dont_traverse


def setup(app):
    app.add_directive('htmlonly', HtmlOnlyDirective, True)
    app.add_directive('latexonly', LatexOnlyDirective, True)
    app.add_node(html_only)
    app.add_node(latex_only)

#~    # This will *really* never see the light of day As it turns out,
#~    # this results in "broken" image nodes since they never get
#~    # processed, so best not to do this.
#~    # app.connect('builder-inited', builder_inited)

#~    # Add visit/depart methods to HTML-Translator:
#~    def visit_perform(self, node):
#~        pass
#~    def depart_perform(self, node):
#~        pass
#~    def visit_ignore(self, node):
#~        node.children = []
#~    def depart_ignore(self, node):
#~        node.children = []

#~    app.add_node(html_only, html=(visit_perform, depart_perform))
#~    app.add_node(html_only, latex=(visit_ignore, depart_ignore))
#~    app.add_node(latex_only, latex=(visit_perform, depart_perform))
#~    app.add_node(latex_only, html=(visit_ignore, depart_ignore))
