cc_library(
    name = "fsm",
    hdrs = ["fsm.h"],
    srcs = ["fsm.cc"],
)

cc_binary(
    name = "regexjit",
    srcs = ["regexjit.cc"],
    deps = [":fsm"],
)

cc_test(
    name = "fsm_test",
    srcs = ["fsm_test.cc"],
    deps = [
        ":fsm",
        "@com_google_gtest//:gtest_main",
    ],
)
