set(example_as_test_suite)
if(${ENABLE_EXAMPLES})
  set(example_as_test_suite
      test/sixlowpan-examples-test-suite.cc
  )
endif()

build_lib(
  LIBNAME sixlowpan
  SOURCE_FILES
    helper/sixlowpan-helper.cc
    model/sixlowpan-header.cc
    model/sixlowpan-net-device.cc
  HEADER_FILES
    helper/sixlowpan-helper.h
    model/sixlowpan-header.h
    model/sixlowpan-net-device.h
  LIBRARIES_TO_LINK ${libinternet}
  TEST_SOURCES
    ${example_as_test_suite}
    test/mock-net-device.cc
    test/sixlowpan-fragmentation-test.cc
    test/sixlowpan-hc1-test.cc
    test/sixlowpan-iphc-stateful-test.cc
    test/sixlowpan-iphc-test.cc
)
