FROM debian:latest

RUN export DEBIAN_FRONTEND=noninteractive && \
    export DEBCONF_NONINTERACTIVE_SEEN=true && \
    apt-get update && \
    apt-get install -y --no-install-suggests --no-install-recommends \
    build-essential \
    ca-certificates \
    check \
    dumb-init \
    git \
    meson \
    pkg-config \
    valgrind && \
    apt-get clean

COPY data ./data
COPY src ./src
COPY include ./include
COPY tests ./tests
COPY tools ./tools
COPY subprojects/*.wrap ./subprojects/
COPY supp ./supp
COPY meson.build meson_options.txt .

RUN CFLAGS="-O0 -DDEBUG -Werror" meson setup builddir
RUN meson compile -C builddir && meson install -C builddir && ldconfig

ENTRYPOINT ["/usr/bin/dumb-init", "--"]
CMD meson test -C builddir; cat builddir/meson-logs/testlog.txt && \
    valgrind \
        --leak-check=full \
        dcm-dump ./data/test_files/sm_image.dcm > /dev/null
