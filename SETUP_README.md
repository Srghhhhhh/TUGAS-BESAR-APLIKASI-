# Ngebuk Warna – Paint App
## Panduan Setup Icon & Splash Screen

---

### Struktur File Baru

```
paint_app/
├── resources/
│   ├── ngebuk_warna.svg     ← asset utama (splash + icon)
│   └── app_icon.ico         ← di-generate oleh generate_icon.py
├── resources.qrc            ← embed SVG ke dalam binary Qt
├── splashscreen.h/.cpp      ← kelas SplashScreen custom
├── main.cpp                 ← entry point (splash + icon logic)
├── app_icon.rc              ← Windows: icon di .exe
├── generate_icon.py         ← helper buat .ico dari SVG
└── CMakeLists.txt           ← sudah ditambah Qt::Svg
```

---

### Langkah 1 – Generate file .ico (Windows)

Lakukan ini **sekali saja** sebelum build pertama:

```bash
# Install dependencies Python
pip install cairosvg pillow

# Generate .ico
cd paint_app
python generate_icon.py
```

File `resources/app_icon.ico` akan terbentuk.
Jika tidak punya Python/cairosvg, skip saja –
icon di window dan taskbar tetap tampil via Qt::Svg,
hanya icon di file .exe yang tidak akan muncul.

---

### Langkah 2 – Build di Qt Creator

1. Buka Qt Creator → **File → Open Project**
2. Pilih `CMakeLists.txt`
3. Klik **Configure Project** (pilih kit Desktop)
4. **Build → Build All** (`Ctrl+B`)
5. **Run** (`Ctrl+R`)

> **Pastikan** modul Qt Svg terinstall.
> Di Qt Maintenance Tool centang: *Qt → Qt X.Y.Z → Additional Libraries → Qt SVG*

---

### Cara Kerja Splash Screen

```
app start
    │
    ▼
SplashScreen tampil      ← SVG di-render jadi QPixmap rounded-corner
    │
    ├─ progress bar animasi (7 tahap loading dengan label)
    │
    ▼
MainWindow di-construct  ← terjadi di belakang layar
    │
    ▼
splash->startFinishTimer()  ← setelah window siap, mulai hitung mundur
    │
    ▼  (progress 100% + 600ms + 400ms delay)
splash->finish(window)   ← Qt fade-out splash, tampilkan MainWindow
```

---

### Kustomisasi

| Yang ingin diubah | Edit di |
|---|---|
| Durasi loading | `splashscreen.cpp` → array `steps` (ubah `delayMs`) |
| Teks loading | `splashscreen.cpp` → field `label` di tiap step |
| Ukuran splash | `main.cpp` → `int splashH = ...` |
| Radius sudut rounded | `main.cpp` → `path.addRoundedRect(..., 28, 28)` |
| Warna progress bar | `splashscreen.cpp` → `drawContents()` → gradient |
