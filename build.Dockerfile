FROM ubuntu:24.04 AS osxcross
ADD --chmod=644 "https://apt.llvm.org/llvm-snapshot.gpg.key" /etc/apt/trusted.gpg.d/apt.llvm.org.asc
WORKDIR /build/osxcross
ADD "https://github.com/tpoechtrager/osxcross.git#27d21e4977c9751d01199c7a226a6faf494c3dd9" .
ADD "https://github.com/joseluisq/macosx-sdks/releases/download/14.5/MacOSX14.5.sdk.tar.xz" ./tarballs/MacOSX14.5.sdk.tar.xz
RUN apt-get update && \
    apt-get install -y ca-certificates
COPY llvm.sources /etc/apt/sources.list.d/llvm.sources
RUN apt update && \
    apt install -y clang-23 git lld-23 make patch xz-utils && \
    ln -s /usr/bin/clang-23 /usr/bin/clang && \
    ln -s /usr/bin/clang++-23 /usr/bin/clang++ && \
    ln -s /usr/bin/ld64.lld-23 /usr/bin/ld64.lld && \
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

ADD --chmod=644 "https://apt.llvm.org/llvm-snapshot.gpg.key" /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN apt-get update && \
    apt-get install -y ca-certificates
COPY llvm.sources /etc/apt/sources.list.d/llvm.sources
RUN apt update && \
    apt install -y clang-23 curl lld-23 meson p7zip-full xz-utils zip && \
    ln -s /usr/bin/clang-23 /usr/bin/clang && \
    curl -fsSL https://deno.land/install.sh | DENO_INSTALL=/usr/local sh
COPY --from=osxcross /osxcross /osxcross
COPY --from=xwin /xwin /xwin

ENV RUSTUP_HOME=/etc/rustup
ENV CARGO_HOME=/etc/cargo
RUN useradd host -u $HOST_UID -o -m && \
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --target x86_64-pc-windows-msvc,aarch64-apple-darwin --no-modify-path && \
    chown -R host:host /etc/cargo
USER host

ENV PATH="/osxcross/bin:/etc/rustup/bin:/etc/cargo/bin:$PATH" \
    LD_LIBRARY_PATH="/osxcross/lib" \
    CARGO_TARGET_X86_64_PC_WINDOWS_MSVC_RUSTFLAGS="-Lnative=/xwin/crt/lib/x86_64 -Lnative=/xwin/sdk/lib/um/x86_64 -Lnative=/xwin/sdk/lib/ucrt/x86_64 -Clinker-plugin-lto" \
    CARGO_TARGET_X86_64_PC_WINDOWS_MSVC_LINKER="lld-link-23" \
    CARGO_TARGET_AARCH64_APPLE_DARWIN_LINKER="oa64-clang"
ENTRYPOINT ["bash"]