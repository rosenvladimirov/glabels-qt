#!/bin/bash
# Build the self-contained python-glabels wheel.
#
# Runs INSIDE an ubuntu:24.04 container (CI job container or `docker run`).
# The wheel bundles everything except glibc/libstdc++/libgcc: Qt 6 and its
# dependencies (glabels/_libs, original sonames), the offscreen platform and
# image-format plugins, the template database and DejaVu/Liberation fonts.
# python/glabels/__init__.py points Qt at them (GLABELS_TEMPLATES_DIR,
# QT_PLUGIN_PATH, FONTCONFIG_FILE) before the extension initialises Qt.
#
# auditwheel is deliberately not used: it renames the Qt libraries
# (libQt6Core-<hash>.so) while the Qt plugins keep looking for the original
# sonames, which ends with two Qt copies in one process.
#
# Tag: cp312-cp312-manylinux_2_39_x86_64 (Ubuntu 24.04 = glibc 2.39, Python 3.12).
#
# usage: build.sh <source dir> <output dir>
set -euo pipefail
SRC=$(realpath "${1:?source dir}")
OUT=$(mkdir -p "${2:?output dir}" && realpath "$2")

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
    build-essential cmake ninja-build patchelf \
    qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools qt6-image-formats-plugins \
    fonts-dejavu-core fonts-liberation \
    libcups2-dev zlib1g-dev python3-dev python3-pip python3-venv >/dev/null
python3 -m venv /tmp/venv
/tmp/venv/bin/pip install -q pybind11 wheel

BUILD=/tmp/build
cmake -S "$SRC" -B "$BUILD" -G Ninja -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release \
      -DPython3_EXECUTABLE=/usr/bin/python3 \
      -Dpybind11_DIR="$(/tmp/venv/bin/python -c 'import pybind11; print(pybind11.get_cmake_dir())')"
cmake --build "$BUILD" --target glabels_ext -j"$(nproc)"

VER=$(sed -n 's/^version *= *"\(.*\)"/\1/p' "$SRC/pyproject.toml")
W=/tmp/wheel; P=$W/glabels
rm -rf "$W"; mkdir -p "$P/_libs" "$P/plugins/platforms" "$P/plugins/imageformats" "$P/fonts"
cp "$SRC"/python/glabels/*.py "$P/"
cp "$BUILD"/python/glabels_ext*.so "$P/"
cp -a "$SRC/templates" "$P/templates"
QP=/usr/lib/x86_64-linux-gnu/qt6/plugins
cp "$QP/platforms/libqoffscreen.so" "$P/plugins/platforms/"
for p in libqsvg.so libqjpeg.so libqgif.so; do
    [ -f "$QP/imageformats/$p" ] && cp "$QP/imageformats/$p" "$P/plugins/imageformats/"
done

SKIP='^(linux-vdso|ld-linux|libc\.|libm\.|libpthread|libdl\.|librt\.|libstdc\+\+|libgcc_s)'
collect() {
    ldd "$1" | awk '/=> \//{print $1" "$3}' | while read -r name path; do
        echo "$name" | grep -Eq "$SKIP" && continue
        if [ ! -e "$P/_libs/$name" ]; then
            cp -L "$path" "$P/_libs/$name"
            collect "$path"
        fi
    done
}
for f in "$P"/glabels_ext*.so "$P"/plugins/*/*.so; do collect "$f"; done
patchelf --set-rpath '$ORIGIN/_libs' "$P"/glabels_ext*.so
for f in "$P"/plugins/*/*.so; do patchelf --set-rpath '$ORIGIN/../../_libs' "$f"; done
for f in "$P"/_libs/*; do patchelf --set-rpath '$ORIGIN' "$f" 2>/dev/null || true; done

cp /usr/share/fonts/truetype/dejavu/DejaVuSans*.ttf "$P/fonts/"
cp /usr/share/fonts/truetype/liberation/LiberationSans-*.ttf "$P/fonts/"
cp /usr/share/doc/fonts-dejavu-core/copyright "$P/fonts/COPYRIGHT-DejaVu"
cp /usr/share/doc/fonts-liberation/copyright "$P/fonts/COPYRIGHT-Liberation"
cat > "$P/fonts.conf" <<'FC'
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
  <include ignore_missing="yes">/etc/fonts/fonts.conf</include>
  <dir prefix="relative">fonts</dir>
  <cachedir prefix="xdg">fontconfig</cachedir>
  <alias binding="same">
    <family>Arial</family>
    <accept><family>Liberation Sans</family></accept>
  </alias>
  <alias>
    <family>sans-serif</family>
    <prefer><family>DejaVu Sans</family></prefer>
  </alias>
</fontconfig>
FC

DI=$W/python_glabels-$VER.dist-info; mkdir -p "$DI"
cp "$SRC/LICENSE" "$DI/LICENSE"
cat > "$DI/METADATA" <<EOF
Metadata-Version: 2.1
Name: python-glabels
Version: $VER
Summary: Python bindings for the gLabels-qt label designer, self-contained (Qt, templates, fonts)
Home-page: https://github.com/rosenvladimirov/glabels-qt
License: GPL-3.0-or-later
Requires-Python: >=3.12,<3.13
Classifier: License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)
Classifier: Operating System :: POSIX :: Linux
Classifier: Programming Language :: Python :: 3.12
Classifier: Topic :: Printing
Description-Content-Type: text/markdown

Python bindings for [gLabels-qt](https://github.com/jimevins/glabels-qt),
packaged with everything they need at run time: Qt 6, the offscreen platform
plugin, the gLabels template database and fonts (DejaVu, Liberation Sans as
Arial). Works on headless servers without Qt installed.

    import glabels
    label = glabels.Label.open("label.glabels")
    label.set_merge_source("Text/Comma/Line1Keys", "data.csv")
    label.render_pdf("labels.pdf")

Not affiliated with the gLabels project. Built on Ubuntu 24.04 (glibc 2.39).
EOF
cat > "$DI/WHEEL" <<EOF
Wheel-Version: 1.0
Generator: packaging/python-wheel/build.sh
Root-Is-Purelib: false
Tag: cp312-cp312-manylinux_2_39_x86_64
EOF
(cd "$W" && /tmp/venv/bin/wheel pack . -d "$OUT")
ls -la "$OUT"/*.whl
