#!/usr/bin/env python3
"""
generate_icon.py
================
Jalankan script ini SEKALI sebelum build untuk menghasilkan:
  resources/app_icon.ico   ← dipakai app_icon.rc (icon .exe Windows)

Kebutuhan:
  pip install cairosvg pillow

Cara pakai:
  cd paint_app
  python generate_icon.py
"""

import sys
import os

def main():
    svg_path = os.path.join("resources", "ngebuk_warna.svg")
    ico_path = os.path.join("resources", "app_icon.ico")

    if not os.path.exists(svg_path):
        print(f"[ERROR] File tidak ditemukan: {svg_path}")
        sys.exit(1)

    try:
        import cairosvg
        from PIL import Image
        import io
    except ImportError:
        print("[ERROR] Modul tidak ditemukan.")
        print("Jalankan: pip install cairosvg pillow")
        sys.exit(1)

    sizes = [256, 128, 64, 48, 32, 24, 16]
    images = []

    print(f"Membaca SVG: {svg_path}")
    for size in sizes:
        png_bytes = cairosvg.svg2png(
            url=svg_path,
            output_width=size,
            output_height=size
        )
        img = Image.open(io.BytesIO(png_bytes)).convert("RGBA")
        images.append(img)
        print(f"  Render {size}x{size} ... OK")

    # Simpan sebagai ICO multi-resolusi
    images[0].save(
        ico_path,
        format="ICO",
        sizes=[(s, s) for s in sizes],
        append_images=images[1:]
    )
    print(f"\n[OK] Icon disimpan ke: {ico_path}")
    print("Sekarang build project Qt seperti biasa.")

if __name__ == "__main__":
    main()
