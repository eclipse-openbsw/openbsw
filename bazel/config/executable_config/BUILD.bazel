"""
Build configuration flag for selecting between application build variants.
This allows targets to define variable dependencies/headers/sources based on the
active build variant (reference_app or unit_test).

- Set via --//bazel/config/executable_config=<value>
- Defaults to "reference_app"
- Overridden to "unit_test" for bazel test commands in .bazelrc
"""
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

string_flag(
    name = "executable_config",
    build_setting_default = "reference_app",
    values = ["reference_app", "unit_test"],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "reference_app",
    flag_values = {":executable_config": "reference_app"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "unit_test",
    flag_values = {":executable_config": "unit_test"},
    visibility = ["//visibility:public"],
)
