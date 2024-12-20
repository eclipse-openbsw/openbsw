# Nix in OpenBSW

Github CI runners don't support vcan. We overcame this by running the integration tests in a virtual machine. Nix infrastructure helps us here because it makes it easy to configure vm based tests.

## General introduction to Nix

Nix takes a unique approach to package management. It requires a declaration of all build inputs and then runs the build in a sandbox to archive reproduceability and determinism (to a very high degree). This effectively makes a package build a pure function without side effects.

These functions used to build packages are called derivations. Packages (derivations) are not limited to only contain a binary or library. It can be basically any directory structure containing arbitrary files.

### Install nix cli

We recommend the [Determinate Nix Installer](https://github.com/DeterminateSystems/nix-installer) to install nix with good defaults and a seamless way of uninstalling nix again.

### Nix the language

The Nix language is a functional programming language used to program and configure everything Nix related. By itself the language is not too big and should be relatively easy to understand if you already have experience with functional languages.
Making an effort to get familiar with the language will flatten the learning curve of other concepts.

Refer to the [Nix Reference Manual](https://nix.dev/manual/nix/2.25/language/) for a language reference.
For people who prefer interactive learning [A tour of Nix](https://nixcloud.io/tour/) is a great resource.

### Nixpkgs

[Nixpkgs](https://github.com/NixOS/nixpkgs) is the standard package repository. It contains all of the packages you would also expect to find in for example in the ubuntu repositories, but packaged with nix.

Use [search.nixos.org](https://search.nixos.org) to search for a package.

### NixOS

NixOS is a linux distribution using Nixpkgs. It is configured with the Nix Language.

### NixOS Tests

NixOS Tests are a super powerful kind of derivation. They allow you to spawn multiple virtual machines, set up networking between them and write a test script in python to test behavior.

### Nix Flakes

Flakes (identified by flake.nix) provide a standard output format of all the nix code of the repository. Additionally flakes help to manage inputs (like nixpkgs or other git repositories) and lock them to a specific version.
With flakes it's possible to run the referenceApp on any system using the following command: `nix run github:eclipse-openbsw/openbsw`.

## Usage

### `nix flake show`

Lists all available attributes.

### `nix flake check`

Runs the integration tests in a VM. They will only work when the VM is accelerated by KVM. Otherwise the tests would run into timeouts and fail.

### `nix run .`

Runs the referenceApp on posix.

### `nix run .#interactiveIntegrationTests`

Runs the integration tests in an interactive mode for debugging
