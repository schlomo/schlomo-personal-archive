#!/bin/bash

echo "🏗️  Building Personal Archive with Thumbnails"
echo "=============================================="

# Check dependencies
echo "🔍 Checking dependencies..."
if ! command -v hugo &> /dev/null; then
    echo "❌ Hugo not found! Please install it first."
    exit 1
fi

if ! command -v convert &> /dev/null && ! command -v magick &> /dev/null; then
    echo "❌ ImageMagick not found! Please install it:"
    echo "  macOS: brew install imagemagick"
    echo "  Linux: sudo apt-get install imagemagick ghostscript"
    exit 1
fi

echo "✅ Hugo found: $(hugo version | head -1)"
echo "✅ ImageMagick found"
echo ""

# Generate thumbnails
echo "🖼️  Generating PDF thumbnails..."
./generate-thumbnails.sh

echo ""
echo "🏗️  Building Hugo site..."
hugo --cleanDestinationDir

echo ""
echo "📊 Build Summary:"
echo "=================="
hugo --quiet | grep -E "Pages|Static files|Total in"

echo ""
echo "✅ Build complete!"
echo "📁 Site built in public/ directory"
echo "🌐 To serve locally: hugo server" 