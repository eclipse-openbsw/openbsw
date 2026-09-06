FROM ubuntu:noble-20251001

RUN for i in 1 2 3; do \
        apt-get update && apt-get install -y \
            clang-format-17 \
            curl \
            git \
            python3-pip \
            python3.10 \
            tar \
        && rm -rf /var/lib/apt/lists/* \
        && break || { echo "Apt install failed, retrying ($i/3)..."; sleep 10; }; \
    done

ARG TARGETARCH

RUN case "${TARGETARCH}" in \
        amd64) treefmt_sha256="ce0863a3a9d73707eb8cb9887c7f97ad3fd4ed506ad0429a6ab67a6290905966"; buildifier_sha256="887377fc64d23a850f4d18a077b5db05b19913f4b99b270d193f3c7334b5a9a7" ;; \
        arm64) treefmt_sha256="7c00ea14f69a95782063e8f86e143bede29c8a6757fea9075ffdfdb6590aa934"; buildifier_sha256="947bf6700d708026b2057b09bea09abbc3cafc15d9ecea35bb3885c4b09ccd04" ;; \
        *) echo "Unsupported TARGETARCH: ${TARGETARCH}" >&2; exit 1 ;; \
    esac \
    && curl --fail --location https://github.com/numtide/treefmt/releases/download/v2.1.0/treefmt_2.1.0_linux_${TARGETARCH}.tar.gz -o treefmt.tar.gz \
    && echo "${treefmt_sha256}  treefmt.tar.gz" | sha256sum -c \
    && tar -xvzf treefmt.tar.gz \
    && install -m 755 treefmt /usr/bin/treefmt \
    && curl --fail --location https://github.com/bazelbuild/buildtools/releases/download/v8.5.1/buildifier-linux-${TARGETARCH} -o buildifier \
    && echo "${buildifier_sha256}  buildifier" | sha256sum -c \
    && install -m 755 buildifier /usr/bin/buildifier \
    && rm LICENSE README.md treefmt treefmt.tar.gz buildifier

RUN pip3 install cmakelang --break-system-packages

ARG user_id=2001

RUN groupadd -g 4996 docker
RUN useradd --shell /bin/bash --uid ${user_id} -g docker -m jenkins

USER jenkins

WORKDIR /home/jenkins
