// Copyright 2026 Accenture.

use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let config = cbindgen::Config::from_file(crate_dir.join("cbindgen.toml"))
        .expect("failed to read cbindgen.toml");
    let output = crate_dir.join("include").join("BswLogger.h");

    // By using `write_to_file` we only rewrite the file when the generated contents differ.
    // This preserves the mtime on a no-op and will then not retrigger any dependent C++ builds.
    cbindgen::Builder::new()
        .with_crate(&crate_dir)
        .with_config(config)
        .generate()
        .map_or_else(
            |error| match error {
                cbindgen::Error::ParseSyntaxError { .. } => {} // ignore in favor of cargo's syntax check
                e => panic!("{:?}", e),
            },
            |bindings| {
                bindings.write_to_file(&output);
            },
        );

    println!("cargo:rerun-if-changed=src");
    println!("cargo:rerun-if-changed=cbindgen.toml");
    println!("cargo:rerun-if-changed=build.rs");
}
