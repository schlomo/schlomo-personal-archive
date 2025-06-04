#!/bin/bash

echo "🖼️  Generating PDF Thumbnails"
echo "=============================="

# Check if ImageMagick is available
if ! command -v convert &> /dev/null; then
    echo "❌ ImageMagick not found! Please install it:"
    echo "  macOS: brew install imagemagick"
    echo "  Linux: sudo apt-get install imagemagick ghostscript"
    exit 1
fi

echo "✅ ImageMagick found: $(convert -version | head -1)"
echo ""

# Create thumbnails directory if it doesn't exist
mkdir -p static/thumbnails

# Counter for generated thumbnails
count=0
total=$(find static -name "*.pdf" -not -path "*/thumbnails/*" | wc -l | tr -d ' ')

echo "📄 Found $total PDF files to process"
echo ""

# Process each PDF file
find static -name "*.pdf" -not -path "*/thumbnails/*" | while read -r pdf_file; do
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
        
        # Generate thumbnail: 200px height, white background, first page only
        if convert -thumbnail x200 -background white -alpha remove "${pdf_file}[0]" "$thumb_path" 2>/dev/null; then
            ((count++))
        else
            echo "❌ Failed to generate thumbnail for $pdf_file"
        fi
    else
        echo "⏭️  $(basename "$pdf_file") (thumbnail up to date)"
    fi
done

echo ""
echo "✅ Generated $count new thumbnails"
echo "📁 Thumbnails saved in static/thumbnails/"
echo ""
echo "💡 To regenerate all thumbnails, delete static/thumbnails/ and run this script again" 