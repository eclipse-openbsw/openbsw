import re
from docutils import nodes
from sphinx.util import logging

logger = logging.getLogger(__name__)

rule_pattern = re.compile(r"^[^-\s]+-[^-\s]+(-[^-\s]+|)$")


def rule_role(_role, rawtext, text, lineno, inliner, _options={}, _content=[]):
    env = inliner.document.settings.env
    rule_id = text.lower()

    if not rule_pattern.match(text):
        logger.warning(
            "%s.rst:%d: Invalid rule pattern %s" % (env.docname, lineno, text)
        )

    if (
        rule_id in env.domaindata["std"]["anonlabels"]
        or rule_id in env.domaindata["std"]["labels"]
    ):
        raise ValueError("Rule %s found twice" % rule_id)

    # make node referencable
    env.domaindata["std"]["anonlabels"][rule_id] = env.docname, rule_id
    env.domaindata["std"]["labels"][rule_id] = env.docname, rule_id, text

    node = nodes.literal(rawtext, "[rule] ")
    node.set_class("highlight")
    node.set_class("highlight-rule")

    self_ref = nodes.reference(text, text, refdocname=env.docname, refid=rule_id)
    node.append(self_ref)

    target_node = nodes.target("", "", node, ids=[rule_id])
    return [target_node], []


def setup(app):
    app.add_role("rule", rule_role)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
