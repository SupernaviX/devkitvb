FROM ubuntu:24.04 AS osxcross
WORKDIR /build/osxcross
ADD "https://github.com/tpoechtrager/osxcross.git#27d21e4977c9751d01199c7a226a6faf494c3dd9" .
ADD "https://github.com/joseluisq/macosx-sdks/releases/download/14.5/MacOSX14.5.sdk.tar.xz" ./tarballs/MacOSX14.5.sdk.tar.xz
RUN apt update && \
    apt install -y clang-18 git lld-18 make patch xz-utils && \
    ln -s /usr/bin/clang-18 /usr/bin/clang && \
    ln -s /usr/bin/clang++-18 /usr/bin/clang++ && \
    ln -s /usr/bin/ld64.lld-18 /usr/bin/ld64.lld && \
    SDK_VERSION=14.5 BUILD_FLAVOR=llvm UNATTENDED=1 ENABLE_COMPILER_RT_INSTALL=1 TARGET_DIR=/osxcross ./build.sh

FROM ubuntu:24.04 AS xwin
WORKDIR /build/xwin
ADD "https://github.com/Jake-Shadle/xwin/releases/download/0.10.0/xwin-0.10.0-x86_64-unknown-linux-musl.tar.gz" ./xwin.tar.gz
RUN apt update && \
    apt install -y ca-certificates && \
    tar -xzvf ./xwin.tar.gz && \
    cd xwin-* && \
    ./xwin --accept-license splat --output /xwin

FROM ubuntu:24.04
ARG HOST_UID

RUN apt update && \
    apt install -y clang curl lld meson p7zip-full xz-utils zip && \
    curl -fsSL https://deno.land/install.sh | DENO_INSTALL=/usr/local sh
COPY --from=osxcross /osxcross /osxcross
COPY --from=xwin /xwin /xwin

RUN useradd host -u $HOST_UID -o
USER host

ENV PATH="/osxcross/bin:$PATH"
ENTRYPOINT ["bash"]