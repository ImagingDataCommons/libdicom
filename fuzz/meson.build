# libFuzzer targets. Built only when -Dfuzzers=true and the compiler
# supports -fsanitize=fuzzer, so a normal build is unaffected.
#
# Local use:
#   CC=clang CFLAGS='-fsanitize=fuzzer-no-link' \
#     meson setup build-fuzz -Dfuzzers=true -Dtests=false \
#     -Db_sanitize=address,undefined -Db_lundef=false
#   ninja -C build-fuzz
#   ./build-fuzz/fuzz/dcm_filehandle_fuzzer fuzz/corpus
#
# OSS-Fuzz passes its own engine flags through $LIB_FUZZING_ENGINE.

fuzz_engine = get_option('fuzzing_engine')

foreach target : ['dcm_filehandle_fuzzer']
  executable(
    target,
    target + '.c',
    dependencies : [libdicom_dep],
    link_args : fuzz_engine != '' ? [fuzz_engine] : ['-fsanitize=fuzzer'],
    install : false,
  )
endforeach
