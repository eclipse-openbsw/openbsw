
alias(
    name = "etl_profile",
    actual = select({
        "//bazel/config/build_config:reference_app": "//executables/referenceApp/etl_profile",
        "//bazel/config/build_config:unit_test": "//executables/unitTest/etl_profile",
        "//conditions:default": "//executables/referenceApp/etl_profile",
    }),
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
