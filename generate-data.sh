#!/usr/bin/env bash
# Requires bash 4+ for readarray

# Source shared dependency checking functions
source "$(dirname "$0")/lib/check-deps.sh"

# Check dependencies
check_hugo
check_yq

echo "📊 Generating presentation data from PDF files..."
echo "================================================="

# Create data directory if it doesn't exist
mkdir -p data

# Create content directories based on years defined in hugo.toml
echo "📁 Creating content directories from hugo.toml configuration..."

# Extract years from Hugo configuration using yq (bash 4+ with readarray)
readarray -t years < <(hugo config --format yaml | yq '.params.years[]' 2>/dev/null)

if [ ${#years[@]} -eq 0 ]; then
    echo "⚠️  No years found in hugo.toml configuration"
    exit 1
fi

echo "📅 Years from config: ${years[*]}"

for year in "${years[@]}"; do
    # Trim whitespace using bash 4+ parameter expansion
    year="${year#"${year%%[![:space:]]*}"}"  # remove leading whitespace
    year="${year%"${year##*[![:space:]]}"}"  # remove trailing whitespace
    
    if [ ! -d "content/$year" ]; then
        echo "   Creating content/$year/"
        mkdir -p "content/$year"
        echo "---" > "content/$year/_index.md"
        echo "---" >> "content/$year/_index.md"
    fi
done

# Validate that all configured year directories exist
echo "🔍 Validating configured year directories..."
missing_dirs=()
for year in "${years[@]}"; do
    # Trim whitespace using bash 4+ parameter expansion
    year="${year#"${year%%[![:space:]]*}"}"  # remove leading whitespace
    year="${year%"${year##*[![:space:]]}"}"  # remove trailing whitespace
    
    year_dir="static/$year"
    if [ ! -d "$year_dir" ]; then
        missing_dirs+=("$year_dir")
    fi
done

if [ ${#missing_dirs[@]} -gt 0 ]; then
    echo "❌ ERROR: The following configured year directories are missing:"
    for dir in "${missing_dirs[@]}"; do
        echo "   📁 $dir"
    done
    echo ""
    echo "💡 Please create these directories and add PDF files, or remove them from hugo.toml"
    exit 1
fi

echo "✅ All configured year directories found"

# Start the YAML file
echo "presentations:" > data/generated.yaml

# Function to extract date from filename
extract_date_from_filename() {
    local filename="$1"
    local year_dir="$2"
    
    # Try to extract year from filename first
    if [[ "$filename" =~ ([0-9]{4}) ]]; then
        local year="${BASH_REMATCH[1]}"
        # Don't guess months - default to January
        echo "$year-01-01"
        return
    fi
    
    # Fallback: use directory year
    if [[ "$year_dir" =~ ^[0-9]{4}$ ]]; then
        echo "$year_dir-01-01"
    elif [[ "$year_dir" == "2005-2016" ]]; then
        echo "2010-01-01"  # Middle of range
    else
        echo "2020-01-01"  # Default fallback
    fi
}

# Process PDF files in configured year directories (all validated to exist)
for year_name in "${years[@]}"; do
    # Trim whitespace using bash 4+ parameter expansion
    year_name="${year_name#"${year_name%%[![:space:]]*}"}"  # remove leading whitespace
    year_name="${year_name%"${year_name##*[![:space:]]}"}"  # remove trailing whitespace
    
    year_dir="static/$year_name"
    echo "🔍 Processing $year_name..."
    
    # Add year section to YAML
    echo "  \"$year_name\":" >> data/generated.yaml
    
    # Find PDF files and process them
    pdf_count=0
    while IFS= read -r -d '' pdf_file; do
        if [ -f "$pdf_file" ]; then
            filename=$(basename "$pdf_file")
            filename_no_ext="${filename%.pdf}"
            
                            # Extract date from filename content or use folder year
                date_from_filename=$(extract_date_from_filename "$filename" "$year_name")
                
                # Check if thumbnail exists
                thumb_path="static/thumbnails/$year_name/${filename_no_ext}.png"
                has_thumbnail="false"
                thumbnail_path=""
                if [ -f "$thumb_path" ]; then
                    has_thumbnail="true"
                    thumbnail_path="thumbnails/$year_name/${filename_no_ext}.png"
                fi
                
                echo "    - title: \"$filename_no_ext\"" >> data/generated.yaml
                echo "      filename: \"$filename\"" >> data/generated.yaml
                echo "      path: \"$year_name/$filename\"" >> data/generated.yaml
                echo "      date: \"$date_from_filename\"" >> data/generated.yaml
                echo "      category: \"presentation\"" >> data/generated.yaml
                echo "      has_thumbnail: $has_thumbnail" >> data/generated.yaml
                if [ "$has_thumbnail" = "true" ]; then
                    echo "      thumbnail: \"$thumbnail_path\"" >> data/generated.yaml
                fi
            
            pdf_count=$((pdf_count + 1))
        fi
    done < <(find "$year_dir" -name "*.pdf" -print0 | sort -z)
    
    echo "   📄 Found $pdf_count PDF files"
done

echo ""
echo "✅ Data generation complete!"
echo "📋 Generated data/generated.yaml with presentation information"
echo "📁 Created content directories for Hugo section pages"
echo ""

echo "📊 Summary:"
echo "==========="

# Count total presentations
total=$(grep -c "^    - title:" data/generated.yaml 2>/dev/null || echo "0")
echo "📄 Total presentations: $total"

# Count by year
for year in $(grep "^  \"[0-9]" data/generated.yaml | sed 's/:$//' | sed 's/^  //' | tr -d '"'); do
    count=$(sed -n "/^  \"$year\":/,/^  \"[0-9]/p" data/generated.yaml | grep -c "^    - title:")
    echo "📅 $year: $count presentations"
done

echo ""
echo "📁 Data file created: data/generated.yaml"
echo "🔄 Run hugo to regenerate RSS feeds with this data" 