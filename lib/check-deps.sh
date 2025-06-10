#!/usr/bin/env bash
# Shared dependency checking functions
# Source this file in other scripts with: source "$(dirname "$0")/lib/check-deps.sh"

# Check if bash version is 4 or higher
if [ "${BASH_VERSION%%.*}" -lt 4 ]; then
    echo "❌ This script requires bash 4 or higher. Current version: $BASH_VERSION"
    exit 1
fi


# Check if Hugo is available
check_hugo() {
    if ! command -v hugo &> /dev/null; then
        echo "❌ Hugo not found! Please install it:"
        echo "  macOS: brew install hugo"
        echo "  Linux: sudo apt-get install hugo"
        echo "  Or download from: https://github.com/gohugoio/hugo/releases"
        exit 1
    fi
    echo "✅ Hugo found: $(hugo version | head -1)"
}

# Check if ImageMagick is available (convert or magick command)
check_imagemagick() {
    if ! command -v convert &> /dev/null && ! command -v magick &> /dev/null; then
        echo "❌ ImageMagick not found! Please install it:"
        echo "  macOS: brew install imagemagick"
        echo "  Linux: sudo apt-get install imagemagick ghostscript"
        exit 1
    fi
    
    if command -v magick &> /dev/null; then
        echo "✅ ImageMagick found: $(magick -version | head -1)"
    elif command -v convert &> /dev/null; then
        echo "✅ ImageMagick found: $(convert -version | head -1)"
    fi
}

# Check if yq is available
check_yq() {
    if ! command -v yq &> /dev/null; then
        echo "❌ yq not found! Please install it:"
        echo "  macOS: brew install yq"
        echo "  Linux: install from https://github.com/mikefarah/yq/releases"
        exit 1
    fi
    echo "✅ yq found: $(yq --version)"
}

# Get ImageMagick convert command (prefers magick over convert to avoid deprecation warnings)
get_imagemagick_convert_cmd() {
    if command -v magick &> /dev/null; then
        echo "magick"
    elif command -v convert &> /dev/null; then
        echo "convert"
    else
        echo ""
        return 1
    fi
} 