# Schlomo Personal Archive

This is a personal archive of presentations, talks, and documents, powered by Hugo static site generator with automatic PDF thumbnail generation.

## Local Development

### Prerequisites

Install Hugo, ImageMagick, and yq (one-time setup):

```bash
# On macOS
brew install hugo imagemagick yq

# On Linux  
sudo apt-get install hugo imagemagick ghostscript
# Install yq from: https://github.com/mikefarah/yq/releases

# Or download Hugo from: https://github.com/gohugoio/hugo/releases
```

### Development Server

```bash
# Generate thumbnails and start development server
./run.sh

# Or manually:
./generate-thumbnails.sh  # Generate PDF thumbnails
hugo server --watch       # Start server with live reloading
```

Visit http://localhost:1313 to see the archive with live reloading.

## Features

- **PDF Thumbnails**: Automatic generation of first-page thumbnails for all PDFs
- **SEO Optimized**: Meta tags, Open Graph, structured data, sitemap
- **Fast Loading**: Static thumbnails with lazy loading
- **Responsive Design**: Mobile-friendly layout
- **Original URLs**: Preserves existing PDF URLs exactly

## File Structure

- `content/` - Markdown files for each year section
- `layouts/` - Hugo templates for generating pages  
- `static/` - PDF files and other static assets (served at root URLs)
- `static/thumbnails/` - Generated PDF thumbnails (auto-created, gitignored)
- `hugo.toml` - Hugo configuration

## Adding New Documents

1. Place PDF files in the appropriate year directory under `static/`
2. Run `./generate-thumbnails.sh` to create thumbnails
3. Hugo automatically detects and lists them on the year pages with thumbnails
4. URLs remain exactly as they are: `/2024/document.pdf`

## Thumbnail Generation

PDF thumbnails are automatically generated using ImageMagick:
- 200px height, maintaining aspect ratio
- PNG format with white background  
- Only first page of each PDF
- Lazy loading for performance
- Incremental updates (only regenerates if PDF is newer)
