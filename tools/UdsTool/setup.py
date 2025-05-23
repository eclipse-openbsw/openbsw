from setuptools import setup, find_packages

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = fh.read()
setup(
    name="UdsTool",
    version="0.0.2",
    author="",
    author_email="",
    license="",
    description="A simple tool to talk to ECUs via UDS (ISO 14229-1)",
    long_description_content_type="text/markdown",
    url="",
    py_modules=["udsTool", "app"],
    packages=find_packages(),
    install_requires=[requirements],
    python_requires=">=3.7",
    classifiers=[
        "Programming Language :: Python :: 3.8",
        "Operating System :: OS Independent",
    ],
    entry_points="""
        [console_scripts]
        udstool=udsTool:start
    """,
)
