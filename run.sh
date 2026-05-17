#!/usr/bin/env bash
# Requires bash 4+ for readarray

set -euo pipefail

# Source shared dependency checking functions
source "$(dirname "$0")/lib/check-deps.sh"

echo "🚀 Starting Hugo development server..."
echo "======================================"

# Check dependencies
echo "🔍 Checking dependencies..."
check_hugo
echo ""

# Generate presentation data
echo "📊 Generating presentation data..."
./generate-data.sh
echo ""

# Generate thumbnails
echo "🖼️  Generating PDF thumbnails..."
./generate-thumbnails.sh
echo ""

# Start the development server
echo "🌐 Starting server at http://localhost:1313"
echo "📁 Watching for file changes..."
echo "🛑 Press Ctrl+C to stop"
echo ""

rm -Rf public
exec hugo server --bind 0.0.0.0 --port 1313 --renderToMemory --buildDrafts --buildFuture --disableFastRender --watch
