#!/usr/bin/env bash
# Requires bash 4+ for readarray

# Source shared dependency checking functions
source "$(dirname "$0")/lib/check-deps.sh"

# Parse command line arguments using getopts
BASE_URL=""
PRODUCTION_BUILD=false

show_help() {
    echo "Usage: $0 [-b <url>] [-p] [-h]"
    echo ""
    echo "Options:"
    echo "  -b <url>    Override the base URL for the site"
    echo "  -p          Enable production optimizations (gc, minify)"
    echo "  -h          Show this help message"
}

while getopts "b:ph" opt; do
    case $opt in
        b)
            BASE_URL="$OPTARG"
            ;;
        p)
            PRODUCTION_BUILD=true
            ;;
        h)
            show_help
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            echo "Use -h for help" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument" >&2
            echo "Use -h for help" >&2
            exit 1
            ;;
    esac
done

echo "🏗️  Building Personal Archive with Thumbnails"
echo "=============================================="

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
echo "🏗️  Building Hugo site..."

# Build Hugo command using bash array (bash 4+ feature)
hugo_args=("hugo" "--cleanDestinationDir")

if [[ -n "$BASE_URL" ]]; then
    hugo_args+=("--baseURL" "$BASE_URL")
    echo "🌐 Using base URL: $BASE_URL"
fi

if [[ "$PRODUCTION_BUILD" == "true" ]]; then
    hugo_args+=("--gc" "--minify")
    echo "⚡ Production optimizations enabled"
fi

# Execute Hugo build using array expansion
"${hugo_args[@]}"

echo ""
echo "✅ Build complete!"
echo "📁 Site built in public/ directory"
if [[ -z "$BASE_URL" ]]; then
    echo "🌐 To serve locally: hugo server"
fi 