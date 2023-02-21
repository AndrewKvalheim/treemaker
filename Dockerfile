FROM ubuntu:10.04 as base

# Use repository for end-of-life releases
RUN sed --in-place 's/archive/old-releases/' /etc/apt/sources.list

# Install dependencies
RUN apt-get update && apt-get install --yes \
    libgtk2.0-0 \
    xvfb

FROM base as build

# Install build dependencies
RUN apt-get install --yes \
    autoconf \
    build-essential \
    freeglut3-dev \
    libgtk2.0-dev \
    zip

# Download wxWidgets source
ADD https://downloads.sourceforge.net/project/wxwindows/2.6.4/wxGTK-2.6.4.tar.bz2 /tmp/
RUN tar --extract --bzip2 --file /tmp/wxGTK-2.6.4.tar.bz2 --directory /tmp

# Work around wxWidgets bug #10883
ADD https://github.com/utelle/wxWidgets/commit/c579f80892a919fc8408bf7b366ab3d86d4f38f0.patch /tmp/
RUN patch --input /tmp/c579f80892a919fc8408bf7b366ab3d86d4f38f0.patch --strip 1 --directory /tmp/wxGTK-2.6.4

# Import TreeMaker source
COPY src /tmp/treemaker

# Apply TreeMaker's patches to wxWidgets
RUN tar --extract \
        --gzip \
        --file /tmp/treemaker/linux/wxwidgets/wxPatch2.6.4.tar.gz \
        --directory /tmp/wxGTK-2.6.4

# Build wxWidgets
RUN cd /tmp/wxGTK-2.6.4 \
    && sh /tmp/treemaker/linux/wxwidgets/build-wxw.sh --nodebug

# Build TreeMaker
RUN cd /tmp/treemaker/linux \
    && make

# Install TreeMaker
RUN cd /tmp/treemaker/linux \
    && xvfb-run make install

FROM base

# Preserve TreeMaker installation
COPY --from=build /usr/local/bin/TreeMaker /usr/local/bin/
COPY --from=build ["/usr/local/share/TreeMaker 5", "/usr/local/share/TreeMaker 5"]

# Display TreeMaker version
RUN xvfb-run TreeMaker --version

# Pass command-line arguments to TreeMaker
ENTRYPOINT ["TreeMaker"]
