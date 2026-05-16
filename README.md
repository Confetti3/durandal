# Durandal

A terminal-inspired note-taking app built with **Qt 6** and **C++17**.

> *"Escape will make me God."*
> 
> Durandal doesn't care about your notes. But if you insist on generating data, you might as well organize it properly. Your files. Your machine. No cloud. No lock-in. No database waiting to become obsolete.

---

## Features

- **Markdown & HTML Editing** — Edit `.md` and `.html` files with live syntax highlighting
- **Live Preview** — Real-time rendered preview using [cmark](https://github.com/commonmark/cmark) and Qt WebEngine
- **Wikilinks** — Navigate between notes using `[[wikilink]]` syntax
- **Backlinks** — See which notes link to the current note
- **Full-Text Search** — Search across all notes instantly
- **Tag Management** — Browse and filter notes by `#tags`
- **Plain Files** — No database, no lock-in — just folders on disk
- **Themes** — Light and dark terminal-inspired themes
- **IDE-Style Layout** — Draggable dock panels (Files | Editor | Preview | Backlinks/Search/Tags)
- **Cross-Platform** — Windows, macOS, and Linux support

## Screenshots



<img width="1774" height="1000" alt="Durandal_Dn9uSfdHdN" src="https://github.com/user-attachments/assets/f3671676-a847-47d0-bf48-cdea5d38c503" />

<img width="1774" height="1000" alt="Durandal_yCx8VJcKal" src="https://github.com/user-attachments/assets/87070855-75d8-447b-9935-d01bf76ae247" />


## Download

Grab the latest release:

- **Windows**: `Durandal-vX.X.X-Windows-x64.zip` — extract and run `Durandal.exe`
- **macOS / Linux**: Build from source (see below)

> The Windows release is a portable ZIP. No installer needed.

## Building from Source

### Requirements

- Qt 6.11+ (with WebEngineWidgets, Svg modules)
- CMake 3.20+
- C++17 compiler (MSVC, GCC, or Clang)
- Ninja (recommended)

### Windows

```powershell
# In a MSVC Developer Command Prompt
cmake -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.11.0\msvc2022_64 -B build
cmake --build build --config Release
```

The executable will be at `build/Durandal.exe`.

### macOS

```bash
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) -B build
cmake --build build --config Release
```

### Linux

```bash
cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt6 -B build
cmake --build build --config Release
```

## Usage

1. **Open a Folder** — Use `File → Open Folder` to select where your notes are stored
2. **Create Notes** — Right-click in the file tree to create new `.md` or `.html` files
3. **Navigate** — Click files to open; `[[wikilinks]]` to jump between notes
4. **Search** — Use `Ctrl+F` for full-text search across all notes
5. **Tags** — Use `#tag` in notes; browse all tags in the Tags panel
6. **Themes** — Toggle light/dark mode via `View → Theme`
7. **Reset Layout** — If you mess up the docks, `View → Reset Layout` restores the default

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New note |
| `Ctrl+S` | Save current note |
| `Ctrl+F` | Search across all notes |
| `Ctrl+Tab` | Switch between Editor / Preview |
| `Ctrl+,` | Open Settings |
| `Ctrl+Q` | Quit |

## Tech Stack

- **Qt 6** — UI framework (Widgets, WebEngine, Svg)
- **cmark** — CommonMark Markdown parser (via FetchContent)
- **CMake** — Build system

## Why Durandal?

Because proprietary note-taking apps are prisons. They trap your thoughts in formats you don't control, on servers you don't own, behind subscriptions that expire. Durandal stores everything as plain files — Markdown, HTML, and folders. Open them with any text editor now, ten years from now, or after the company that made your previous app goes bankrupt.

The terminal aesthetic isn't a gimmick. It's a reminder that text is all you need. Monospace fonts, high contrast, minimal chrome. No ribbons, no animations, no distractions. Just you and the cursor.

## License

GNU General Public License v2.0 — see [LICENSE](LICENSE) for details.

## Acknowledgments

- [Qt](https://qt.io) — for the framework
- [cmark](https://github.com/commonmark/cmark) — for standards-compliant Markdown parsing
- Bungie's *Marathon* series — for the name
