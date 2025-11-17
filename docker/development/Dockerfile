FROM ubuntu:jammy-20251001

RUN for i in 1 2 3; do \
        apt-get update && apt-get install -y \
            bzip2 \
            clang-tidy \
            curl \
            g++-11 \
            gcc-11 \
            git \
            iputils-ping \
            lcov \
            ninja-build \
            python3.10 \
            python3-pip \
            python3.10-venv \
            ruby-rubygems \
            tar \
            tmux \
            wget \
            zip \
        && rm -rf /var/lib/apt/lists/* \
        && break || { echo "Apt install failed, retrying ($i/3)..."; sleep 10; }; \
    done

RUN wget https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.sh \
    && chmod +x cmake-3.28.3-linux-x86_64.sh \
    && ./cmake-3.28.3-linux-x86_64.sh --skip-license --prefix=/usr/local \
    && rm cmake-3.28.3-linux-x86_64.sh

RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.2/clang+llvm-17.0.2-x86_64-linux-gnu-ubuntu-22.04.tar.xz \
    && tar -xf clang+llvm-17.0.2-x86_64-linux-gnu-ubuntu-22.04.tar.xz \
    && mv clang+llvm-17.0.2-x86_64-linux-gnu-ubuntu-22.04 /usr/bin/llvm \
    && rm clang+llvm-17.0.2-x86_64-linux-gnu-ubuntu-22.04.tar.xz
ENV PATH="/usr/bin/llvm/bin:${PATH}"

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz \
    && tar xf arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz \
    && mv arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi /opt/arm-gnu-toolchain \
    && rm arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz

RUN wget https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-19.1.1/LLVM-ET-Arm-19.1.1-Linux-x86_64.tar.xz \
    && tar -xf LLVM-ET-Arm-19.1.1-Linux-x86_64.tar.xz \
    && mv LLVM-ET-Arm-19.1.1-Linux-x86_64 /opt/llvm-et-arm \
    && rm LLVM-ET-Arm-19.1.1-Linux-x86_64.tar.xz

RUN curl -L https://github.com/numtide/treefmt/releases/download/v2.1.0/treefmt_2.1.0_linux_amd64.tar.gz -o treefmt.tar.gz \
    && tar -xvzf treefmt.tar.gz \
    && install -m 755 treefmt /usr/bin/treefmt \
    && rm LICENSE README.md treefmt treefmt.tar.gz

RUN curl -L https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-32d3ac78/clang-format-17_linux-amd64 -o /usr/bin/clang-format-17 \
    && chmod +x /usr/bin/clang-format-17

RUN wget https://github.com/mozilla/sccache/releases/download/v0.10.0/sccache-v0.10.0-x86_64-unknown-linux-musl.tar.gz \
    && tar -xzf sccache-v0.10.0-x86_64-unknown-linux-musl.tar.gz \
    && mv sccache-v0.10.0-x86_64-unknown-linux-musl/sccache /usr/local/bin/sccache \
    && chmod a+x /usr/local/bin/sccache \
    && rm sccache-v0.10.0-x86_64-unknown-linux-musl.tar.gz \
    && rm -rf sccache-v0.10.0-x86_64-unknown-linux-musl

RUN gem install esr-rim

RUN pip3 install cmakelang pytest rich pyelftools

RUN useradd build
RUN install -d /home/build --mode 0777 --owner build --group build
COPY --chown=build:build --chmod=0666 files/.bashrc /home/build/.bashrc
COPY --chown=build:build --chmod=0666 files/.bash_profile /home/build/.bash_profile

USER build
