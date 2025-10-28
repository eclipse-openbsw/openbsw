import os
from datetime import datetime
import jinja2
from puncover import collector
from puncover.renderers import HTMLRenderer, register_jinja_filters
from puncover.gcc_tools import GCCTools
from puncover.builders import ElfBuilder
import shutil
from jinja2 import Undefined
import re
from pathlib import Path
import hashlib

# Constants
TYPE = "type"
TYPE_FILE = "file"
TYPE_FOLDER = "folder"
FILE = "file"
FOLDER = "folder"
SYMBOLS = "symbols"
FILES = "files"
SUB_FOLDERS = "sub_folders"
COLLAPSED_SUB_FOLDERS = "collapsed_sub_folders"
NAME = "name"
ADDRESS = "address"
PATH = "path"
KEY_OUTPUT_FILE_NAME = "output_file_name"


class LocalHTMLRenderer(HTMLRenderer):
    @jinja2.pass_context
    def assembly_filter(self, context, value):
        url_for = context.get("url_for", None)

        def linked_symbol_name(name):
            display_name = self.display_name_for_symbol_name(name)
            symbol = self.collector.symbol(name, False)
            if not symbol:
                return name
            if url_for:
                url = url_for("symbol", symbol=symbol)
            else:
                current_file_path = context.get("output_file_name", None)
                url = self.url_for(
                    "symbol",
                    current_file_path=current_file_path,
                    symbol=symbol,
                    from_assembly=True,
                )
            return '<a href="%s">%s</a>' % (url, display_name)

        value = str(value)
        pattern = re.compile(r"<(\w+)")
        s = pattern.sub(lambda match: "<" + linked_symbol_name(match.group(1)), value)
        pattern = re.compile(r"^(_.*)\(\):$")

        def display_name_for_label(match):
            display_name = self.display_name_for_symbol_name(match.group(1))
            if display_name.endswith(")"):
                return display_name + ":"
            else:
                return display_name + "():"

        s = pattern.sub(display_name_for_label, s)
        return s

    def __init__(self, collector):
        super().__init__(collector)
        self.template_vars.update(
            {
                "now": datetime.now(),
            }
        )
        self.symbols_by_address = {
            int(symbol[ADDRESS], 16): symbol
            for symbol in collector.symbols.values()
            if ADDRESS in symbol
        }
        template_loader = jinja2.FileSystemLoader(searchpath="./templates")
        self.template_env = jinja2.Environment(loader=template_loader, autoescape=True)
        register_jinja_filters(self.template_env)
        self.template_env.filters["bytes"] = self.custom_bytes_filter
        self.template_env.filters["assembly"] = self.assembly_filter

    def render_template(self, template_name, file_path, **kwargs):
        sanitized_file_path = file_path.replace("<", "").replace(">", "")
        self.template_vars.update(kwargs)
        self.template_vars[KEY_OUTPUT_FILE_NAME] = sanitized_file_path
        self.template_env.globals["url_for"] = lambda endpoint, **vals: self.url_for(
            endpoint, file_path, **vals
        )
        template = self.template_env.get_template(template_name)
        try:
            rendered_html = template.render(**self.template_vars)
        except Exception as e:
            print(
                f"Template rendering error for {template_name} at {sanitized_file_path}: {e}"
            )
            raise
        output_path = os.path.join("output", sanitized_file_path + ".html")
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, "w") as f:
            f.write(rendered_html)
        return rendered_html

    def custom_bytes_filter(self, value):
        if isinstance(value, Undefined) or value is None:
            return ""
        if not isinstance(value, (int, float)):
            return ""
        return "{:,}".format(value)

    def get_hashed_name(self, mangled_name):
        return hashlib.md5(mangled_name.encode("utf-8")).hexdigest()

    def get_file_path(self, file_entry):
        """Helper method to extract the file path from a FILE entry, whether it's a Path object or a dictionary."""
        if isinstance(file_entry, Path):
            return str(file_entry).replace(os.sep, "/")
        elif isinstance(file_entry, dict) and PATH in file_entry:
            return str(file_entry[PATH]).replace(os.sep, "/")
        else:
            # print(f"Warning: Invalid file entry: {file_entry}")
            return "unknown/unknown"

    def url_for(self, endpoint, current_file_path=None, from_assembly=False, **values):
        def relative_path(target):
            if not current_file_path:
                return f"./{target}"
            current_full_path = os.path.join("output", current_file_path)
            target_full_path = os.path.join("output", target)
            current_dir = os.path.dirname(current_full_path)
            rel_path = os.path.relpath(target_full_path, current_dir).replace(
                os.sep, "/"
            )
            return rel_path if rel_path.startswith(".") else f"./{rel_path}"

        def sanitize_path(path):
            return path.replace("<", "").replace(">", "")

        if endpoint == "base":
            return relative_path("index.html")
        elif endpoint == "index":
            return relative_path("index.html")
        elif endpoint == "all":
            return relative_path("all.html")
        elif endpoint == "rack":
            return relative_path("rack.html")
        elif endpoint == "folder":
            folder_path = str(values["folder"][PATH]).replace(os.sep, "/")
            return sanitize_path(relative_path(f"{folder_path}.html"))
        elif endpoint == "file":
            file_path = str(values["file"][PATH]).replace(os.sep, "/")
            return sanitize_path(relative_path(f"{file_path}.html"))
        elif endpoint == "symbol":
            symbol = values["symbol"]
            if ADDRESS not in symbol:
                return "#"
            mangled_name = symbol.get("name", f"symbol_{symbol[ADDRESS]}")
            hashed_name = self.get_hashed_name(mangled_name)
            if from_assembly:
                file_path = self.get_file_path(symbol[FILE])
                return sanitize_path(
                    relative_path(f"{file_path}/{hashed_name}.html")
                ).removeprefix(".")
            else:
                if FILE not in symbol:
                    file_path = "external"
                else:
                    file_path = self.get_file_path(symbol[FILE])
                return sanitize_path(relative_path(f"{file_path}/{hashed_name}.html"))
        elif endpoint == "static":
            filename = values.get("filename", "")
            return relative_path(os.path.join("static", filename))
        return "#"


def generate_html_output(gcc_tools_base):
    src_root = os.path.abspath("../source") + os.sep
    build_dir = os.path.abspath("../../build/s32k148-freertos-gcc") + os.sep
    elf_file = os.path.join(
        build_dir,
        "executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf",
    )
    builder = create_builder(
        gcc_tools_base, elf_file=elf_file, src_root=src_root, su_dir=build_dir
    )
    builder.build_if_needed()
    renderer = LocalHTMLRenderer(builder.collector)
    output_dir = "output"
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)
    static_source = "./static"
    static_dest = os.path.join("output", "static")
    if os.path.exists(static_source):
        shutil.copytree(static_source, static_dest)
    else:
        print(f"Warning: Static folder {static_source} not found")
    all_files = builder.collector.all_files()
    root_folders = builder.collector.root_folders()
    root_files = [file for file in all_files if file[FOLDER] is None]
    all_symbols = builder.collector.symbols.values()
    all_folders = builder.collector.all_folders()
    renderer.render_template(
        "overview.html.jinja", "index", root_folders=root_folders, root_files=root_files
    )
    for symbol in all_symbols:
        if FILE not in symbol or ADDRESS not in symbol:
            continue
        file_path = renderer.get_file_path(symbol[FILE])
        mangled_name = symbol.get("name", f"symbol_{symbol[ADDRESS]}")
        hashed_name = renderer.get_hashed_name(mangled_name)
        symbol_file_name = f"{file_path}/{hashed_name}"
        renderer.render_template("symbol.html.jinja", symbol_file_name, symbol=symbol)
    for file in all_files:
        file_path = str(file.get(PATH, "")).replace(os.sep, "/")
        renderer.render_template("file.html.jinja", file_path, file=file)
    for folder in all_folders:
        try:
            folder_path = str(folder.get(PATH, "")).replace(os.sep, "/")
            renderer.render_template("folder.html.jinja", folder_path, folder=folder)
        except Exception as e:
            print(f"Error rendering folder {folder_path}: {e}")
    renderer.render_template("all_symbols.html.jinja", "all", symbols=all_symbols)
    renderer.render_template("rack.html.jinja", "rack")


def create_builder(gcc_base_filename, elf_file=None, su_dir=None, src_root=None):
    c = collector.Collector(GCCTools(gcc_base_filename))
    if elf_file:
        return ElfBuilder(c, src_root, elf_file, su_dir)
    else:
        raise Exception("Unable to configure builder for collector")


def get_arm_tools_prefix_path():
    obj_dump = shutil.which("/usr/bin/gcc-arm-none-eabi/bin/arm-none-eabi-objdump")
    if not obj_dump:
        return None
    gcc_tools_base_dir = os.path.dirname(os.path.dirname(obj_dump))
    assert gcc_tools_base_dir, "Unable to find gcc tools base dir from {}".format(
        obj_dump
    )
    return os.path.join(gcc_tools_base_dir, "bin/arm-none-eabi-")


def main():
    gcc_tools_base = get_arm_tools_prefix_path()
    if gcc_tools_base is None:
        exit(1)
    generate_html_output(gcc_tools_base)


if __name__ == "__main__":
    main()
