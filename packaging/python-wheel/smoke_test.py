"""Smoke test of an installed python-glabels wheel.

Run in a clean environment (no Qt, no fonts): opens a template with a merge
field, fills it from CSV, renders a PDF and checks that the merged TEXT is in
it. An empty ${field} or a missing font still produces a "valid" PDF, so the
page size alone proves nothing.
"""

import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))

os.chdir(tempfile.gettempdir())  # never import from a source checkout
import glabels  # noqa: E402

assert "site-packages" in glabels.__file__ or "dist-packages" in glabels.__file__, glabels.__file__
n_templates = len(glabels.list_templates())
assert n_templates > 1000, n_templates

label = glabels.Label.open(os.path.join(HERE, "smoke", "label.glabels"))
label.set_merge_source("Text/Comma/Line1Keys", os.path.join(HERE, "smoke", "label.csv"))
out = os.path.join(tempfile.gettempdir(), "smoke.pdf")
label.render_pdf(out)

from pypdf import PdfReader  # noqa: E402

text = " ".join(PdfReader(out).pages[0].extract_text().split())
for expected in ("Примерен партньор", "34111"):
    assert expected in text, (expected, text)
print("OK:", glabels.version(), n_templates, "templates |", text)
sys.exit(0)
