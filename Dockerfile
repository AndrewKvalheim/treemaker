FROM ubuntu:10.04

# Use repository for end-of-life releases
RUN sed --in-place 's/archive/old-releases/' /etc/apt/sources.list

# Install dependencies
RUN apt-get update && apt-get install --yes \
    autoconf \
    build-essential \
    freeglut3-dev \
    libgtk2.0-dev \
    xvfb \
    zip

# Download wxWidgets source
ADD https://downloads.sourceforge.net/project/wxwindows/2.6.4/wxGTK-2.6.4.tar.bz2 /tmp/
RUN tar --extract --bzip2 --file /tmp/wxGTK-2.6.4.tar.bz2 --directory /tmp

# Work around wxWidgets bug #10883
RUN curl 'https://trac.wxwidgets.org/changeset/61009/svn-wx?format=diff&new=61009' \
    | patch --strip 4 --directory /tmp/wxGTK-2.6.4

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

# Display TreeMaker version
RUN xvfb-run TreeMaker --version

# Pass command-line arguments to TreeMaker
ENTRYPOINT ["TreeMaker"]
