#!/bin/bash

echo "🚀 Starting Hugo development server..."

# Check if Hugo is installed
if ! command -v hugo &> /dev/null; then
    echo "❌ Hugo is not installed!"
    echo ""
    echo "📦 Install Hugo:"
    echo "  On macOS:    brew install hugo imagemagick"
    echo "  On Linux:    sudo apt-get install hugo imagemagick ghostscript"
    echo "  Or download: https://github.com/gohugoio/hugo/releases"
    exit 1
fi

echo "✅ Hugo found: $(hugo version)"

# Check for ImageMagick and generate thumbnails if available
if command -v convert &> /dev/null || command -v magick &> /dev/null; then
    echo "🖼️  Generating PDF thumbnails..."
    ./generate-thumbnails.sh --quiet 2>/dev/null || ./generate-thumbnails.sh
else
    echo "⚠️  ImageMagick not found - PDF thumbnails disabled"
    echo "   Install with: brew install imagemagick (macOS) or sudo apt-get install imagemagick (Linux)"
fi

echo ""

# Start the development server
echo "🌐 Starting server at http://localhost:1313"
echo "📁 Watching for file changes..."
echo "🛑 Press Ctrl+C to stop"
echo ""

rm -Rf public
exec hugo server --bind 0.0.0.0 --port 1313 --renderToMemory --buildDrafts --buildFuture --disableFastRender --watch
