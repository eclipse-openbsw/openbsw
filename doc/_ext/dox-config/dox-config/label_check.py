from docutils import nodes
from sphinx.util import logging

logger = logging.getLogger(__name__)


def add_refid(std, docname, location, ref_id):
    std["label_location"][ref_id] = location
    if docname not in std["labels_in_document"]:
        std["labels_in_document"][docname] = {ref_id}
    else:
        std["labels_in_document"][docname].add(ref_id)


def collect_labels(app, doctree):
    std = app.env.domaindata["std"]
    docname = app.env.docname
    for node in doctree.traverse(nodes.target):
        ref_id = node.get("refid")
        # ignore duplicated labels for a single env, this will be checked by Sphinx internally
        if ref_id:
            location = docname + ".rst"
            if ref_id in std["label_location"]:
                location_other = std["label_location"][ref_id]
                if location_other != location:
                    logger.warning(
                        f"label {ref_id} defined in {location} and {location_other}"
                    )
            else:
                add_refid(std, docname, location, ref_id)


def purge_labels(_app, env, docname):
    std = env.domaindata["std"]
    if "labels_in_document" in std:
        if docname in std["labels_in_document"] and "label_location" in std:
            for ref_id in std["labels_in_document"][docname]:
                std["label_location"].pop(ref_id, None)
        std["labels_in_document"].pop(docname, None)


def prepare_labels(app, _env, _docnames):
    std = app.env.domaindata["std"]
    if "labels_in_document" not in std:
        std["labels_in_document"] = {}
    if "label_location" not in std:
        std["label_location"] = {}


def merge_labels_parallel_build(_app, env, docnames, other):
    std = env.domaindata["std"]
    std_other = other.domaindata["std"]

    for docname in docnames:
        if docname not in std_other["labels_in_document"]:
            # no labels to merge
            continue
        for ref_id in std_other["labels_in_document"][docname]:
            location_other = std_other["label_location"][ref_id]
            if ref_id in std["label_location"]:
                location = std["label_location"][ref_id]
                if location_other != location:
                    logger.warning(
                        f"label {ref_id} defined in {location} and {location_other}"
                    )
                # else: label already there, can happen when rebuilding the documentation
            else:
                add_refid(std, docname, location_other, ref_id)
