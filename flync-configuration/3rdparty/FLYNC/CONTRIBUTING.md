# CONTRIBUTING TO FLYNC

## Introduction

Thank you for your interest in contributing to the FLYNC project. This project is licensed under the Apache License v2.0, and we welcome contributions from the community to enhance its functionality, improve documentation, and fix issues.

This document outlines the process for contributing to the project. By following these guidelines, you help ensure a smooth and collaborative development process.

All contributions must comply with the **Apache-2.0 license**. Ensure you understand the terms of the license before contributing.

## Getting Started

Before contributing, familiarize yourself with the project:

- **Repository**: The source code is hosted at <https://github.com/Technica-Engineering/FLYNC>.
- **Documentation**: Read the main documentation to understand the project's functionality and structure.
- **Issue Tracker**: Check the issue tracker for open issues, bugs, or feature requests.

To contribute, you will need:

- **Python 3.12 to 3.15**
- Git for version control
- A code editor (e.g., VS Code, PyCharm)
- Familiarity with the project dependencies (e.g., ``pydantic``, ``pyyaml``, ``pytest``)

To get a glance of the project dependencies, check:
```bash
uv tree
```

## Contribution Workflow

Follow these steps to contribute to the FLYNC project:

1. **Fork the Repository**:

   - Fork the repository (see Github on how to fork).

2. **Create a Feature Branch**:

   - Clone your fork of FLYNC, if not done already.

   - Create a new branch for your contribution with a meaningful name and structure using this format:
    ```bash
    <type>/<description>
    ```
    - `main`: The main development branch (e.g., main, master, or develop)
    - `feature/` (or `feat/`): For new features (e.g., `feature/add-login-page`, `feat/add-login-page`)
    - `bugfix/` (or `fix/`): For bug fixes (e.g., `bugfix/fix-header-bug`, `fix/header-bug`)
    - `hotfix/`: For urgent fixes (e.g., `hotfix/security-patch`)
    - `release/`: For branches preparing a release (e.g., `release/v1.2.0`)
    - `chore/`: For non-code tasks like dependency, docs updates (e.g., `chore/update-dependencies`)

    *See the full [guideline for conventional branch naming](https://conventional-branch.github.io/).*

    To create a branch use this command:
     ```bash
     git checkout -b <type>/<description>
     ```

3. **Make Changes**:

   - Implement your changes, ensuring they align with the project's coding standards.
   - Update or add tests to cover your changes (if applicable).
   - Update documentation by adding examples or extending the API reference (if applicable).

4. **Commit Changes**:

   - Write clear, concise commit messages following the format:
     ```
     scope: short description

     Detailed Description
     Reference to issue
     ```

     Example:
     ```
     Validator: add support for new command

     Added a new command to validate SOME/IP configurations with enhanced error reporting.

     Closes: #123456
     ```

5. **Push Changes**:

   - Push your branch to your fork:
    ```bash
    git push origin your-branch-name
    ```

6. **Submit a Pull Request**:

   - Open a pull request (PR) against the main repository's ``main`` branch.
   - Provide a clear description of your changes, referencing any related issues (e.g., ``Closes: #123``).
   - Ensure your PR passes all automated checks (e.g., linting, tests).

7. **Code Review**:

   - The project maintainers will review your PR.
   - Address any feedback by making additional commits to the same branch.
   - Once approved, your changes will be merged.

  > **HINT**: Keep your fork in sync with the upstream repository by periodically pulling changes:

  ```bash
  git remote add upstream main-repo-url
  git fetch upstream
  git rebase upstream/main
  ```


## Coding Standards

The FLYNC project adheres to strict coding standards to ensure consistency, maintainability, and readability across the codebase.

These standards are particularly important when introducing new parts to the model, such as new classes, methods, or configuration validations, to ensure seamless integration with the existing architecture.

This document outlines the coding standards for contributing to the FLYNC project, with a focus on Python conventions, Pydantic model development, and best practices for automotive configuration management.

All contributors must follow these guidelines to maintain the project's quality and compatibility with its license.

### General Python conventions

The FLYNC project follows [PEP 8](https://www.python.org/dev/peps/pep-0008/) for Python code style, with specific conventions tailored to the project's needs.

Key guidelines include:

- **Indentation**: Use 4 spaces per indentation level. Do not use tabs.
- **Line Length**: Limit lines to 149 characters for readability.
- **Imports**:
    - Group imports in the following order: standard library, third-party, local project modules.
    - Use explicit imports (e.g., ``from typing import List`` instead of ``import typing``).
- **Naming Conventions**:
    - **Classes**: Use **PascalCase** (e.g., ``SwitchPort``, ``MulticastGroup``).
    - **Methods and Functions**: Use **snake_case** (e.g., ``validate_config``, ``get_config``).
    - **Variables**: Use **snake_case** for variables and attributes (e.g., ``silicon_port_no``, ``default_vlan_id``).
    - **Constants**: Use **UPPER_SNAKE_CASE** for constants (e.g., ``INSTANCES``).
    - **Private Attributes**: Prefix with a single underscore for protected attributes (e.g., ``_mdi_config``) and use ``PrivateAttr`` for Pydantic private attributes.
- **Docstrings**: Follow [PEP 257](https://www.python.org/dev/peps/pep-0257/) for docstrings. Use triple double-quotes (``"""``) and include:
    - A brief description of the class, method, or function.
    - Parameters, return values, and exceptions (if applicable) in a structured format.
    - Example for a class:

      ``` python
        class VirtualControllerInterface(FLYNCBaseModel):
        """
        Represents a virtual interface on a controller.

        Parameters
        ----------
        name : str
            Name of the virtual interface.

        vlanid : int
            VLAN identifier in the range 0-4095.

        addresses : list of \
        :class:`~flync.model.flync_4_ecu.sockets.IPv4AddressEndpoint` or \
        :class:`~flync.model.flync_4_ecu.sockets.IPv6AddressEndpoint`
            Assigned IPv4 and IPv6 address endpoints.

        multicast : list of :class:`IPv4Address` or :class:`IPv6Address` \
        or str, optional
            Allowed multicast addresses.
        """

### Pydantic Model Development

FLYNC relies heavily on [Pydantic](https://docs.pydantic.dev/latest/) for data validation and model definition.

When introducing new parts to the model (e.g., new classes or fields), adhere to the following standards:

- **Class Definition**:
    - Inherit from ``FLYNCBaseModel`` (a project-specific base class) to ensure consistent validation and configuration.
    - Use **PascalCase** for class names (e.g., ``SwitchPort``, ``VLANEntry``).
    - Declare a ``name`` field directly when the model needs a name; uniqueness, if required, is enforced at the owning parent model:

    ```python
    class SwitchPort(FLYNCBaseModel):
        name: str
        # ... other fields
    ```

    The owning parent model enforces uniqueness via a ``@model_validator``:

    ```python
    from pydantic import model_validator

    class Switch(FLYNCBaseModel):
        ports: List[SwitchPort]
        # ... other fields

        @model_validator(mode="after")
        def validate_unique_port_names(self):
            common_validators.validate_list_items_unique(
                [p.name for p in self.ports],
                "Switch Ports (name)",
            )
            return self
    ```

- **Model Configuration**:
    - Set ``model_config`` as desired. Some defaults are already set in ``FLYNCBaseModel``:

    ```python
    FLYNCBaseModel.model_config: ClassVar[ConfigDict] = {'extra': 'forbid'}
    ```
    Configuration for the model, should be a dictionary conforming to [ConfigDict][pydantic.config.ConfigDict].

- **Fields**:
    - Use type hints for all fields, leveraging ``typing`` module types (e.g., ``List``, ``Optional``).
    - For constrained fields, use Pydantic's ``Field`` with appropriate constraints (e.g., ``ge``, ``le``):

    ```python

        default_vlan_id: int = Field(..., ge=0, le=4095)
    ```

    - For union types with discriminators (e.g., different PHY types), use pythons pipe operator:

    ```python
        mii_config: Optional[MII | RMII | SGMII | RGMII | XFI] = pydantic.Field(default=None, discriminator="type")
    ```

    - For private attributes, use Pydantic's ``PrivateAttr``:

    ```python
        _mdi_config: BASET1 | BASET1S | BASET = PrivateAttr()
    ```

    - The field defines how the workspace will look like for this model (read/write). So make sure you're using the correct types and annotations.
      Check out `field_annotations <guidelines/field_annotations.rst>` for more details on the annotations.

    ```python
        info: Annotated["MetadataECU", External(output_structure=OutputStrategy.SINGLE_FILE)] = pydantic.Field()
    ```

- **Validators**:
    - Preferable validators for the fields are validator classes like `BeforeValidator`, `AfterValidator`, `PlainValidator` etc. See [annotated validators](https://docs.pydantic.dev/latest/concepts/validators/#using-the-annotated-pattern).
    - Use Pydantic's ``@field_validator`` and ``@model_validator`` for field-specific and model-wide validations, respectively.
    - Place validators in the appropriate mode (``before``, ``after``) based on the validation needs.
    - Raise descriptive ``ValueError`` exceptions with context:

    ```python
        @field_validator('multicast', mode='after')
        @classmethod
        def validate_multicast_ip(cls, val_list):
            """
            Validates that all provided IP addresses are multicast.

            Raises:
                ValueError: If any address is not a multicast address.
            """
            for val in val_list:
                if val and not val.is_multicast:
                    raise ValueError("Address must be a Multicast Address. Unicast provided")
            return val_list
    ```
    - Ensure validators are specific to the model and avoid generic logic that could belong in a parent class.

- **Documentation**:
    - Provide detailed docstrings for all models, including parameters, constraints, and usage notes.
    - Reference related classes using RST-style links (e.g., ``mii_config (:class:`~flync.model.flync_4_ecu.phy.MII`)``).


### Error Handling and Logging

- **Logging**:
    - Use the ``logging`` module for debugging and informational messages.
    - Log relevant events, such as file parsing or validation steps:

    ```python
        logger.info(f"Parsed Service Interface: {service_interface}")
    ```
- **Error propagation**
    - Use the `err_minor(msg, **ctx)`, `err_major(msg, **ctx)`, `err_fatal(msg, **ctx)` to raise PydanticCustomError with respective severity.

## File and Directory Structure

When introducing new model components, ensure they align with the project's directory structure:

- Place new model classes in the appropriate module (e.g., ``src/flync/model/flync_4_ecu/`` for ECU-related models, ``src/flync/model/flync_4_tsn/`` for TSN-related models).
- Use descriptive file names in **snake_case** (e.g., ``feature1_part1.py``).
- Maintain a consistent module hierarchy, mirroring the configuration structure (e.g., ``controllers/``, ``ports/``).

> **HINT**: When adding a new model, check for compatibility with existing YAML schemas to avoid breaking existing configurations.

## Testing

All new model components must include tests to validate their behavior:

- **Test Framework**: Use ``pytest`` for unit and integration tests.
- **Test Location**: Place tests in a ``tests/`` directory, mirroring the main codebase structure (e.g., ``tests/flync_4_ecu/test_switch_port.py``).
- **Test Coverage**:
    - Test all fields and validators, including edge cases.
    - Test error conditions (e.g., invalid VLAN IDs, missing required fields).
- Run tests using the following command from the main directory:

```python
    pytest tests/
```

> **WARNING**: Ensure all tests pass before submitting a pull request. Untested code will not be accepted.


## Documentation

Update documentation for any changes that affect usage or contribution guidelines.

- **Code Documentation**:
    - Provide comprehensive docstrings for all classes, methods, and functions.
    - Use RST-style references for cross-linking to other classes or modules (e.g., `:class:~flync.model.flync_4_ecu.phy.MII`.
    - Include usage notes for complex configurations (e.g., conditions for optional fields).

- **Project Documentation**:
    - Update any RST file to reflect new model components or changes to existing ones.
    - Use reStructuredText (RST) for all documentation files deployed to the documentation.
    - Update this ``CONTRIBUTING.md`` if new contribution processes are introduced.

> **NOTE**: Preview RST files using a tool like Sphinx or an RST viewer to ensure correct rendering.


## Issue Reporting

If you encounter bugs or have feature requests:

- Check the issue tracker to avoid duplicates.
- Open a new issue with:
  - A clear title (e.g., ``Bug: YAML parsing fails with invalid syntax``).
  - A detailed description, including steps to reproduce, expected behavior, and actual behavior.
  - Relevant logs or screenshots.
  - Your environment (e.g., Python version, OS).

> **ATTENTION**: Provide as much detail as possible in issue reports to help maintainers diagnose and resolve issues quickly.

## Feature Requests

We welcome ideas that improve usability, performance, or integration with automotive toolchains.

When suggesting a feature, please describe:

- The problem you are trying to solve
- Your expected workflow
- How the feature would integrate into SDV development processes

## Security Issues

If you discover a potential security issue, please **do not** report it publicly in the issue tracker.

Instead, contact the maintainers directly via [flync@technica-engineering.de](mailto:flync@technica-engineering.de)

This allows us to investigate and address the issue responsibly.


## Code of Conduct

The project maintainers are committed to fostering an inclusive and respectful community. All contributors must adhere to the following principles:

- Be respectful and professional in all interactions.
- Avoid discriminatory language or behavior.
- Collaborate constructively, providing helpful feedback.

Violations may result in removal from the project.


License
-------

The FLYNC project is licensed under **Apache-2.0**. By contributing, you agree that your contributions will be licensed under its regulations.
