# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

# -- Project information -----------------------------------------------------

project = 'SOFEA'
copyright = ' 2020 Dow, Inc. and Exponent'
author = 'John Buonagurio, Steven Cryer, Ian van Wesenbeeck'

version = '4.1.5'
release = '4.1.5-ALPHA'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinx.ext.mathjax',
    'sphinx.ext.ifconfig',
    'sphinx.ext.githubpages',
    'sphinx.ext.todo',
]

templates_path = ['_templates']

source_suffix = '.rst'

master_doc = 'index'

language = None

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

pygments_style = None

# -- Options for HTML output -------------------------------------------------

html_theme = 'alabaster'

html_theme_options = {
    'logo': 'sofea-logo-256.png',
    'description': 'SOil Fumigant Exposure Assessment Modeling System',
    'page_width': '940px',
    'sidebar_width': '220px',
    'github_button': 'true',
    'github_banner': 'false',
    'github_user': 'sofea-model',
    'github_repo': 'sofea',
}

html_static_path = ['_static']

html_sidebars = {
    '**': [
        'about.html',
        'navigation.html',
        'relations.html',
        'searchbox.html'
    ]
}

# -- Options for HTMLHelp output ---------------------------------------------

htmlhelp_basename = 'sofeadoc'

# -- Options for LaTeX output ------------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    'pointsize': '11pt',

    # Additional stuff for the LaTeX preamble.
    'preamble': '\\numberwithin{equation}{section}',

    # Latex figure (float) alignment
    'figure_align': 'htbp',
}

latex_documents = [
    (master_doc, 'sofea.tex', 'SOFEA Documentation',
     'John Buonagurio, Steven Cryer, Ian van Wesenbeeck', 'manual'),
]

# -- Options for manual page output ------------------------------------------

man_pages = [
    (master_doc, 'sofea', 'SOFEA Documentation',
     [author], 1)
]

# -- Options for Texinfo output ----------------------------------------------

texinfo_documents = [
    (master_doc, 'sofea', 'SOFEA Documentation',
     author, 'SOFEA', 'SOil Fumigant Exposure Assessment Modeling System',
     'Miscellaneous'),
]

# -- Options for Epub output -------------------------------------------------

epub_title = project

# epub_identifier = ''

# epub_uid = ''

epub_exclude_files = ['search.html']

# -- Extension configuration -------------------------------------------------
