#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build_dir="$repo_root/build"
assets_root="$repo_root/Orbiter"
xr2_assets_root=""
output="$repo_root/dist/out/linux/XRVessels-linux.tar.xz"

usage() {
    cat <<'EOF'
Usage: package-linux.sh [options]

Builds one Orbiter-root install archive containing the Linux XR1, XR2, and XR5
modules plus the vessel data available in this repository.

Options:
  --build-dir PATH     CMake build directory (default: ./build)
  --assets-root PATH   Root containing Config, Images, Meshes, Scenarios,
                       and Textures to package (default: ./Orbiter)
  --xr2-assets PATH    Optional Orbiter root with licensed XR2Ravenstar
                       meshes and textures; these are not in this repository
  --output PATH        Archive path (default: dist/out/linux/XRVessels-linux.tar.xz)
  -h, --help           Show this help
EOF
}

while (($#)); do
    case "$1" in
        --build-dir|--assets-root|--xr2-assets|--output)
            (($# >= 2)) || { echo "Missing value for $1" >&2; exit 2; }
            option=$1
            value=$2
            case "$option" in
                --build-dir) build_dir=$value ;;
                --assets-root) assets_root=$value ;;
                --xr2-assets) xr2_assets_root=$value ;;
                --output) output=$value ;;
            esac
            shift 2
            ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

build_dir=$(realpath -m -- "$build_dir")
assets_root=$(realpath -m -- "$assets_root")
output=$(realpath -m -- "$output")
if [[ -n "$xr2_assets_root" ]]; then
    xr2_assets_root=$(realpath -m -- "$xr2_assets_root")
fi

modules=(DeltaGliderXR1 XR2Ravenstar XR5Vanguard)
for module in "${modules[@]}"; do
    if [[ ! -f "$build_dir/Modules/$module.so" ]]; then
        echo "Missing built module: $build_dir/Modules/$module.so" >&2
        echo "Build the package targets first (DeltaGliderXR1 XR2Ravenstar XR5Vanguard)." >&2
        exit 1
    fi
done

for path in Config Images Meshes Scenarios Textures; do
    [[ -d "$assets_root/$path" ]] || {
        echo "Missing asset directory: $assets_root/$path" >&2
        exit 1
    }
done
for path in \
    Config/Vessels/DeltagliderXR1.cfg \
    Config/Vessels/XR2Ravenstar.cfg \
    Config/Vessels/XR5Vanguard.cfg \
    Meshes/DG-XR1/deltaglider-xr1.msh \
    Meshes/XR5Vanguard/XR5Vanguard.msh; do
    [[ -f "$assets_root/$path" ]] || {
        echo "Missing required vessel data: $assets_root/$path" >&2
        exit 1
    }
done

mkdir -p -- "$(dirname -- "$output")"
stage=$(mktemp -d)
trap 'rm -rf -- "$stage"' EXIT
install_root="$stage/install"
mkdir -p "$install_root/Modules" "$install_root/Doc"

for module in "${modules[@]}"; do
    install -m 0755 -- "$build_dir/Modules/$module.so" "$install_root/Modules/$module.so"
done

for path in Config Images Meshes Scenarios Textures; do
    cp -a -- "$assets_root/$path" "$install_root/"
done
# XR3 is not part of the upstream all-vessels distribution yet.
rm -f -- "$install_root/Config/Vessels/XR3Phoenix.cfg" \
    "$install_root/Config/XR3PhoenixPrefs.cfg"
rm -rf -- "$install_root/Meshes/XR3Phoenix" \
    "$install_root/Textures/XR3Phoenix" \
    "$install_root/Scenarios/XR3 Phoenix"
if [[ -d "$assets_root/Doc" ]]; then
    cp -a -- "$assets_root/Doc/." "$install_root/Doc/"
fi

cp -- "$repo_root/XRVessels/DeltaGliderXR1/DeltaGliderXR1Prefs.cfg" "$install_root/Config/"
cp -- "$repo_root/XRVessels/XR2Ravenstar/XR2RavenstarPrefs.cfg" "$install_root/Config/"
cp -- "$repo_root/XRVessels/XR5Vanguard/XR5VanguardPrefs.cfg" "$install_root/Config/"
cp -- "$repo_root/XRVessels/XR Flight Operations Manual.pdf" "$install_root/Doc/"

if [[ -n "$xr2_assets_root" ]]; then
    for path in Meshes/XR2Ravenstar Textures/XR2Ravenstar; do
        [[ -d "$xr2_assets_root/$path" ]] || {
            echo "Missing XR2 asset directory: $xr2_assets_root/$path" >&2
            exit 1
        }
        mkdir -p "$install_root/$(dirname -- "$path")"
        cp -a -- "$xr2_assets_root/$path" "$install_root/$(dirname -- "$path")/"
    done
else
    echo "Note: XR2 meshes and textures were omitted (proprietary assets are not in this repository)." >&2
fi

tar -C "$install_root" -cJf "$output" .
echo "Created $output"
