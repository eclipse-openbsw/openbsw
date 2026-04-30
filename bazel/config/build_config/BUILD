
"""
Build configuration flag for selecting between application build variants.
This allows targets to define variable dependencies/headers/sources based on the
active build variant (reference_app or unit_test).

Set via:
- --//bazel/config/build_config:build_config=<value>
- Defaults to "reference_app" in general
- Overridden to "unit_test" for bazel test commands in .bazelrc

NOTE: When adding variance using select() based on this configuration flag:
- Selects should be defined inside the consuming package's BUILD file. This avoids indirection,
   keeps variant logic readable and provides a single point of change when adding new variants.
- If the config value is not logically tied to a package or used in many packages it may instead be
  defined as a common config in bazel/build/config/<config_name>:<config_name>
- Selects should either inlcude a default or raise an specific error.
"""
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

string_flag(
    name = "build_config",
    build_setting_default = "reference_app",
    values = ["reference_app", "unit_test"],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "reference_app",
    flag_values = {":build_config": "reference_app"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "unit_test",
    flag_values = {":build_config": "unit_test"},
    visibility = ["//visibility:public"],
)
