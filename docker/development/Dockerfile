FROM ubuntu:noble-20251001

RUN apt-get update && apt-get install -y \
        bzip2 \
        clang-17 \
        clang-format-17 \
        clang-tidy-17 \
        cmake \
        curl \
        g++-11 \
        gcc-11 \
        git \
        lcov \
        ninja-build \
        python-is-python3 \
        python3 \
        python3-pip \
        python3-venv \
        ruby-rubygems \
        tar \
        tmux \
        wget \
        zip

RUN wget -O gcc-arm.tar.xz https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz \
    && echo "8f6903f8ceb084d9227b9ef991490413014d991874a1e34074443c2a72b14dbd gcc-arm.tar.xz" | sha256sum -c \
    && mkdir -p /opt/arm-gnu-toolchain \
    && tar -xf gcc-arm.tar.xz -C /opt/arm-gnu-toolchain --strip-components 1 \
    && rm gcc-arm.tar.xz

RUN wget -O clang-arm.tar.xz https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/releases/download/release-19.1.1/LLVM-ET-Arm-19.1.1-Linux-x86_64.tar.xz \
    && echo "f659c625302f6d3fb50f040f748206f6fd6bb1fc7e398057dd2deaf1c1f5e8d1 clang-arm.tar.xz" | sha256sum -c \
    && mkdir -p /opt/llvm-et-arm \
    && tar -xf clang-arm.tar.xz -C /opt/llvm-et-arm --strip-components 1 \
    && rm clang-arm.tar.xz

RUN wget -O treefmt.tar.gz https://github.com/numtide/treefmt/releases/download/v2.1.0/treefmt_2.1.0_linux_amd64.tar.gz \
    && echo "ce0863a3a9d73707eb8cb9887c7f97ad3fd4ed506ad0429a6ab67a6290905966 treefmt.tar.gz" | sha256sum -c \
    && tar -xf treefmt.tar.gz \
    && install -m 755 treefmt /usr/bin/treefmt \
    && rm LICENSE README.md treefmt treefmt.tar.gz

RUN gem install esr-rim

RUN python3 -m venv /opt/venv
ENV PATH=/opt/venv/bin:${PATH}
RUN --mount=type=bind,source=files/requirements.lock,target=/tmp/requirements.lock pip3 install -r /tmp/requirements.lock

RUN useradd build
RUN install -d /home/build --mode 0777 --owner build --group build
COPY --chown=build:build --chmod=0666 files/.bashrc /home/build/.bashrc
COPY --chown=build:build --chmod=0666 files/.bash_profile /home/build/.bash_profile

USER build
