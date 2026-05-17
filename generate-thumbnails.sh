#!/usr/bin/env bash
# Requires bash 4+ for readarray

set -o pipefail

# Source shared dependency checking functions
source "$(dirname "$0")/lib/check-deps.sh"

# Check dependencies
check_imagemagick

echo "🖼️  Generating PDF Thumbnails"
echo "=============================="

# Get the correct ImageMagick command
convert_cmd=$(get_imagemagick_convert_cmd)

# Run conversion with argument order matching the selected command.
# ImageMagick 7 `magick` expects the input image before many operators.
run_thumbnail_convert() {
    local input_file="$1"
    local output_file="$2"
    local err_file="$3"

    if [[ "$convert_cmd" == "magick" ]]; then
        "$convert_cmd" "$input_file" -thumbnail x200 -background white -alpha remove "$output_file" 2>"$err_file"
    else
        "$convert_cmd" -thumbnail x200 -background white -alpha remove "$input_file" "$output_file" 2>"$err_file"
    fi
}

echo ""

# Create thumbnails directory if it doesn't exist
mkdir -p static/thumbnails

# Counter for generated thumbnails
count=0
failed=0
total=$(find static -name "*.pdf" -not -path "*/thumbnails/*" -print0 | sort -z | tr -cd '\0' | wc -c)

echo "📄 Found $total PDF files to process"
echo ""

# Process each PDF file using safe filename handling
while IFS= read -r -d '' pdf_file; do
    # Extract relative path from static/
    rel_path="${pdf_file#static/}"
    
    # Create thumbnail filename  
    thumb_path="static/thumbnails/${rel_path%.pdf}.png"
    
    # Create thumbnail directory if needed
    thumb_dir=$(dirname "$thumb_path")
    mkdir -p "$thumb_dir"
    
    # Generate thumbnail only if it doesn't exist or PDF is newer
    if [[ ! -f "$thumb_path" ]] || [[ "$pdf_file" -nt "$thumb_path" ]]; then
        echo "🖼️  $(basename "$pdf_file") → thumbnails/${rel_path%.pdf}.png"
        
        # Generate thumbnail: 200px height, white background, first page only.
        convert_err="$(mktemp "/tmp/thumb-convert.XXXXXX.log")"

        if run_thumbnail_convert "${pdf_file}[0]" "$thumb_path" "$convert_err"; then
            ((count++))
        else
            ((failed++))
            echo "❌ Failed to generate thumbnail for $pdf_file"
            echo "   ImageMagick error output:"
            sed 's/^/   /' "$convert_err"
        fi
        rm -f "$convert_err"
    else
        echo "⏭️  $(basename "$pdf_file") (thumbnail up to date)"
    fi
done < <(find static -name "*.pdf" -not -path "*/thumbnails/*" -print0 | sort -z)

echo ""
echo "✅ Generated $count new thumbnails"
if [[ "$failed" -gt 0 ]]; then
    echo "❌ Failed to generate $failed thumbnails"
    echo "🛑 Aborting because thumbnail generation must succeed"
    exit 1
fi
echo "📁 Thumbnails saved in static/thumbnails/"
echo ""
echo "💡 To regenerate all thumbnails, delete static/thumbnails/ and run this script again"