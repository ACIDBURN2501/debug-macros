# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import shutil
import datetime
import sphinx_rtd_theme

# -- Helper Methods ----------------------------------------------------------

def have_command(cmd):
    """Search ``cmd`` in the ``PATH`` environment.

    If found, return True.
    If not found, return False.
    """
    return shutil.which(cmd) is not None

# -- General Configuration ---------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
# needs_sphinx = '3.4.3'

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Debug Module'
copyright = 'MIT'
copyright = str(datetime.datetime.today().year)
author = 'Acid'

# The version info for the project you're documenting, acts as replacement for
# |version|, also used in various other places throughout the
# built documents.
#
# The following code tries to extract the information by reading the README,
# when Sphinx is run directly (e.g. by Read the Docs).
try:
    readme_version = None
    for line in open('../../../README.md'):
        if "Version:" in line:
            key, val = [x.strip() for x in line.split(':', 2)]
            readme_version = val
            break
except IOError as e:
    pass
finally:
    if readme_version:
        version = readme_version
    else:
        version = "unknown version"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration
extensions = ['sphinx.ext.autodoc', 'sphinx.ext.doctest',
              'sphinx.ext.intersphinx', 'sphinx.ext.todo',
              'sphinx.ext.coverage', 'sphinx.ext.mathjax',
              'sphinx.ext.ifconfig', 'sphinx.ext.viewcode',
              'breathe', 'sphinx_rtd_theme', 'sphinx.ext.graphviz']

breathe_projects = {"Debug Module": "../doxygen"}
breathe_default_project = "Debug Module"

master_doc = 'index'

html_sidebars = {'**': ['searchbox.html', 'globaltoc.html',
                        'relations.html', 'sourcelink.html', ]}
html_theme_options = {
'version_selector': True,}

templates_path = ['_templates']
exclude_patterns = []

primary_domain = 'c'


latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    'papersize': 'a4paper',

    # The font size ('10pt', '11pt' or '12pt').
    'pointsize': '11pt',

    # Latex figure (float) alignment
    #'figure_align': 'htbp',

    # Don't mangle with UTF-8 chars
    'inputenc': '',
    'utf8extra': '',

    # Set document margins
    'sphinxsetup': '''
        hmargin=0.5in, vmargin=1in,
        parsedliteralwraps=true,
        verbatimhintsturnover=false,
    ''',

    #
    # Some of our authors are fond of deep nesting; tell latex to
    # cope.
    #
    'maxlistdepth': '10',

    # For CJK One-half spacing, need to be in front of hyperref
    'extrapackages': r'\usepackage{setspace}',
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

current_version = version
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
