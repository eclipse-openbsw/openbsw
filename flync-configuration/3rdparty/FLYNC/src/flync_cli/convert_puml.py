"""Renders PlantUML files to SVG/HTML, downloading the PlantUML jar on demand."""

import argparse
import subprocess
import sys
import urllib.request
from pathlib import Path

PLANTUML_JAR_URL = "https://github.com/plantuml/plantuml/releases/latest/download/plantuml.jar"


def download_jar(target_path: Path):
    """
    Downloads the PlantUML JAR file if it doesn't exist.
    """
    print(f"PlantUML engine not found. Downloading from {PLANTUML_JAR_URL}...")
    try:
        # We use a user agent to avoid being blocked by some servers
        req = urllib.request.Request(PLANTUML_JAR_URL, headers={"User-Agent": "Mozilla/5.0"})
        with urllib.request.urlopen(req) as response, open(target_path, "wb") as out_file:
            data = response.read()
            out_file.write(data)
        print(f"Successfully downloaded to {target_path}")
        return True
    except Exception as e:
        print(f"Error downloading PlantUML: {e}")
        return False


def convert_puml(input_file, output_format):  # NOSONAR
    """
    Converts a PlantUML file to the specified format using the local plantuml.jar.
    """
    input_path = Path(input_file).resolve()
    if not input_path.exists():
        print(f"Error: Input file '{input_file}' does not exist.")
        return False

    jar_path = Path("plantuml.jar").resolve()
    if not jar_path.exists() and not download_jar(jar_path):
        print("Error: Could not obtain 'plantuml.jar'. Please download it manually to the project root.")
        return False

    # Define the output file path based on the input name and format
    # PlantUML by default names the output based on the input filename

    cmd = ["java", "-jar", str(jar_path)]

    if output_format == "pdf":
        cmd.append("-tpdf")
    elif output_format == "html":
        # For HTML, we'll first generate an SVG and then wrap it
        cmd.append("-tsvg")
    elif output_format == "svg":
        cmd.append("-tsvg")
    else:
        cmd.append("-tpng")

    cmd.append(str(input_path))

    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"PlantUML Error:\n{result.stderr}")
            return False

        # Post-process for HTML format
        if output_format == "html":
            svg_file = input_path.with_suffix(".svg")
            if svg_file.exists():
                wrap_svg_in_html(svg_file)
                print(f"Success: Created {input_path.with_suffix('.html')}")
                # Optional: remove the intermediate SVG? Usually good to keep.
            else:
                print("Error: SVG generation failed, cannot create HTML.")
                return False
        else:
            print(f"Success: Created {input_path.with_suffix('.' + output_format)}")

        return True
    except Exception as e:
        print(f"An error occurred: {str(e)}")
        return False


def wrap_svg_in_html(svg_path):
    """
    Wraps the SVG content in a high-quality, scrollable HTML viewer.
    """
    html_path = svg_path.with_suffix(".html")
    with open(svg_path, "r", encoding="utf-8") as f:
        svg_content = f.read()

    # We use a wrapper with overflow: auto to allow scrolling for large diagrams
    # We also inject some CSS to ensure the SVG doesn't get clipped or scaled down.

    html_template = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>FLYNC System UML Viewer</title>
    <style>
        :root {{
            --primary: #2c3e50;
            --bg: #f8f9fa;
            --card: #ffffff;
            --accent: #3498db;
        }}
        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background-color: var(--bg);
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            height: 100vh;
            overflow: hidden;
        }}
        header {{
            background-color: var(--primary);
            color: white;
            padding: 0.75rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            z-index: 100;
        }}
        h1 {{
            margin: 0;
            font-size: 1.1rem;
            font-weight: 600;
            letter-spacing: -0.02em;
        }}
        .controls {{
            display: flex;
            gap: 0.5rem;
            align-items: center;
        }}
        .zoom-level {{
            font-size: 0.85rem;
            min-width: 3rem;
            text-align: center;
            font-variant-numeric: tabular-nums;
            background: rgba(255,255,255,0.1);
            padding: 0.3rem;
            border-radius: 4px;
        }}
        .btn {{
            background: rgba(255,255,255,0.1);
            border: 1px solid rgba(255,255,255,0.2);
            color: white;
            padding: 0.4rem 0.8rem;
            border-radius: 4px;
            cursor: pointer;
            font-size: 0.85rem;
            font-weight: 500;
            transition: all 0.2s;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .btn:hover {{
            background: rgba(255,255,255,0.2);
            border-color: rgba(255,255,255,0.4);
        }}
        .btn-accent {{
            background: var(--accent);
            border-color: var(--accent);
        }}
        .btn-accent:hover {{
            background: #2980b9;
        }}
        .viewer-container {{
            flex: 1;
            overflow: auto;
            padding: 3rem;
            display: block;
            background-image:
                radial-gradient(#d1d5db 1px, transparent 1px);
            background-size: 30px 30px;
            scroll-behavior: smooth;
        }}
        .diagram-wrapper {{
            background: white;
            padding: 4rem;
            border-radius: 12px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.08);
            display: inline-block;
            min-width: min-content;
            transform-origin: top left;
            transition: transform 0.1s ease-out;
        }}
        svg {{
            display: block;
            width: auto !important;
            height: auto !important;
            max-width: none !important;
            max-height: none !important;
        }}
        footer {{
            background: white;
            padding: 0.5rem 2rem;
            border-top: 1px solid #e5e7eb;
            font-size: 0.75rem;
            color: #6b7280;
            text-align: right;
            z-index: 100;
        }}
    </style>
</head>
<body>
    <header>
        <h1>FLYNC Architecture Explorer</h1>
        <div class="controls">
            <button class="btn" onclick="changeZoom(-0.1)" title="Zoom Out">−</button>
            <span class="zoom-level" id="zoomDisplay">100%</span>
            <button class="btn" onclick="changeZoom(0.1)" title="Zoom In">+</button>
            <button class="btn" onclick="resetZoom()" style="margin-left: 10px;">Reset</button>
            <button class="btn btn-accent" onclick="location.reload()" style="margin-left: 10px;">Reload</button>
        </div>
    </header>
    <main class="viewer-container" id="viewer">
        <div class="diagram-wrapper" id="wrapper">
            {svg_content}
        </div>
    </main>
    <footer>
        Generated by FLYNC CLI Tools &bull; {svg_path.name}
    </footer>

    <script>
        let scale = 1.0;
        const wrapper = document.getElementById('wrapper');
        const display = document.getElementById('zoomDisplay');

        function changeZoom(delta) {{
            scale = Math.max(0.1, Math.min(5.0, scale + delta));
            updateZoom();
        }}

        function resetZoom() {{
            scale = 1.0;
            updateZoom();
        }}

        function updateZoom() {{
            wrapper.style.transform = `scale(${{scale}})`;
            display.innerText = `${{Math.round(scale * 100)}}%`;
        }}

        // Mouse wheel zoom with Ctrl
        document.getElementById('viewer').addEventListener('wheel', (e) => {{
            if (e.ctrlKey) {{
                e.preventDefault();
                const delta = e.deltaY > 0 ? -0.1 : 0.1;
                changeZoom(delta);
            }}
        }}, {{ passive: false }});
    </script>
</body>
</html>
"""
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html_template)


def main():
    """Entry point for the puml-to-html CLI: parse arguments and convert a .puml file to the chosen format."""
    parser = argparse.ArgumentParser(description="Convert PlantUML files to PDF or HTML.")
    parser.add_argument("input", help="Path to the .puml file")
    parser.add_argument("--format", choices=["pdf", "html", "svg", "png"], default="html", help="Output format (default: html)")

    args = parser.parse_args()

    if convert_puml(args.input, args.format):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()
