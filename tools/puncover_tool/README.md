# Puncover tool

## Installation

```bash
pipx install poetry
```

## Running the Tool
Option 1: Run using run.py

```bash
cd tools/puncover_tool
python run.py
```

This will:

Install any missing dependencies using Poetry
Execute the script and generate the report

Option 2: If all dependencies are already installed, you can run the main script directly using:

```bash
poetry run puncover_tool
```

This will execute the ``generate_html.py`` script through Poetry's virtual environment.

The output can be viewed located at:

```bash
tools/puncover_tool/output/index.html
```