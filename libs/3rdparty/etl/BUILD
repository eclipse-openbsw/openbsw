
alias(
    name = "etl_profile_default",
    actual = select({
        "//bazel/config/executable_config:reference_app": "//executables/referenceApp/etl_profile",
        "//bazel/config/executable_config:unit_test": "//executables/unitTest/etl_profile",
        "//conditions:default": "//executables/referenceApp/etl_profile",
    }),
)

label_flag(
    name = "etl_profile",
    build_setting_default = ":etl_profile_default",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "etl",
    hdrs = glob(["include/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        ":etl_profile",
    ],
)
