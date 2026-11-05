# AGENTS.md

Instructions for AI coding agents working on this repository. Read `README.md` for the overview;
this file is about keeping the archive intact.

## What this is

Schlomo Schapiro's archive of public talks and documents, <https://archive.schlomo.schapiro.org/>:
PDFs grouped by year, with generated first-page thumbnails. Hugo static site, built by GitHub
Actions and hosted on GitHub Pages (deploys only from `main`).

## Map

| What | Where |
|---|---|
| The documents (served at `/<year>/<file>`) | `static/<year>/` |
| Which years exist, in display order | `years` in `hugo.toml` |
| Color palette | `[params.colors]` in `hugo.toml` |
| `<head>` (meta, link previews, JSON-LD), page shell | `layouts/_default/baseof.html` |
| Home page (year grid) / year pages (file list) | `layouts/index.html` / `layouts/_default/section.html` |
| Atom feed with Media RSS | `layouts/index.xml` (output format "RSS", content is Atom) |
| Styles | `assets/css/style.css` |
| Build pipeline | `build.sh` → `generate-data.sh` + `generate-thumbnails.sh` + Hugo |
| CI / deploy | `.github/workflows/gh-pages.yml`, `.github/dependabot.yml` |
| Old university pages (legacy, keep as is) | `static/huji/` |

Generated, never committed: `data/generated.yaml`, `content/<year>/`, `static/thumbnails/`,
`public/`.

## Rules: don't break these

1. **URLs are permanent.** Every file under `static/` is linked from elsewhere (talks, slides,
   other sites). Never rename, move or delete a published file; add new ones instead.
2. **New documents:** put the PDF in `static/<year>/`. A new year also needs its entry in
   `years` (`hugo.toml`, newest first) and the directory to exist; `generate-data.sh` fails
   otherwise.
3. **File size:** GitHub rejects files over 100 MB, and GitHub Pages sites are limited to 1 GB in
   total. Check before adding large recordings.
4. **One source per fact.** Colors live only in `[params.colors]` (`hugo.toml`), named like the CSS
   variables; never hard-code a color in CSS or templates. Hugo lowercases parameter keys, so
   use lowercase names with hyphens. Shadows and the gradient are derived from the palette in
   `style.css`.
5. **Site name:** `siteName` ("Schlomo Schapiro") is what Google shows as the site name. It must
   stay short; the long title belongs in `alternateName`.
6. **Templates reference assets through Hugo** (`resources.Get … | fingerprint`) so they get
   content-hashed URLs. `static/` is for files whose URL must never change (documents, favicons,
   `og-image.jpg`).
7. **Build tools:** Hugo is pinned in the workflow (`HUGO_VERSION` + `HUGO_SHA256`; bump both
   together, the checksum comes from Hugo's release `checksums.txt`). Thumbnails need ImageMagick
   and Ghostscript; `generate-data.sh` needs `yq`.

## Verify a change

```bash
./build.sh          # full build: data, thumbnails, Hugo; must print no ERROR or WARN
./run.sh            # live preview on http://localhost:1313
```

Then check that the home page lists every year from `hugo.toml`, a year page shows every PDF
with its thumbnail, and existing PDF URLs still resolve. The workflow builds every pushed
branch, so a PR shows a green `build` check before merging.
