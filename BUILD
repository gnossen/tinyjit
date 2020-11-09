cc_library(
    name = "assembly_segment",
    hdrs = ["assembly_segment.h"],
    srcs = ["assembly_segment.cc"],
)

cc_test(
    name = "assembly_segment_test",
    srcs = ["assembly_segment_test.cc"],
    deps = [
        ":assembly_segment",
        "@com_google_gtest//:gtest_main",
    ],
)

cc_library(
    name = "fsm",
    hdrs = ["fsm.h"],
    srcs = ["fsm.cc"],
    deps = [":assembly_segment"],
)

cc_test(
    name = "fsm_test",
    srcs = ["fsm_test.cc"],
    deps = [
        ":fsm",
        "@com_google_gtest//:gtest_main",
    ],
)

cc_binary(
    name = "regexjit",
    srcs = ["regexjit.cc"],
    deps = [
      ":fsm",
      ":assembly_segment",
    ],
)

