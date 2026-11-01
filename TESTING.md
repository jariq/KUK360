# KUK360 release testing

Manual checks before publishing a new version. Run them with the distribution that will be published (Windows MSI and zip, macOS DMG, Linux AppImage), not with a development build. Platform-specific steps are marked **Win**, **Mac** and **Linux**.

Before testing, back up the settings (Windows `HKCU\Software\KUK360\KUK360`, macOS `defaults export com.kuk360.KUK360 <file>`, Linux `~/.config/KUK360/KUK360.conf`) and restore them afterwards (macOS `defaults import com.kuk360.KUK360 <file>`; copying the plist file doesn't work, `cfprefsd` caches the settings).

## Test data

A test folder with copies, never the original photos:
- `KUK360-Photo-Sample.jpg` (panorama with GPano XMP)
- a panorama without XMP (e.g. the sample re-saved by an editor that drops metadata)
- an ordinary photo (JPEG) in portrait and landscape
- one image of each other format: PNG, BMP, GIF, TIFF, WebP
- files named `pano2.jpg` and `pano10.jpg` (natural order)
- a broken file with a supported extension (e.g. a text file renamed to `broken.jpg`)
- a large panorama (at least 16000 px wide) if available

## Installation and start

- [ ] **Win** The MSI installs without warnings, the signature is valid (installer and `KUK360.exe`: Properties > Digital Signatures).
- [ ] **Win** The MSI upgrades the previous version (2.x or 3.x): only one KUK360 in "Installed apps" afterwards, the Start menu shortcut works.
- [ ] **Win** The portable zip extracts and `KUK360.exe` starts by double-click.
- [ ] **Mac** The DMG opens without a Gatekeeper warning, the app copied to Applications starts without a warning.
- [ ] **Linux** The AppImage starts after `chmod +x`, both in a Wayland and an X11 session.
- [ ] Help > About shows the new version and the license notices.
- [ ] `LICENSE.txt`, `THIRD_PARTY_NOTICES.txt` and the `licenses` directory with one directory per component are next to the application (**Mac** `KUK360.app/Contents/Resources`, **Linux** `usr/share/doc/KUK360` in the AppImage, see `--appimage-extract`).
- [ ] The start page is shown, the window has the size and position from the last session.

## Opening files

- [ ] File > Open (Ctrl + O): the dialog starts in the folder of the last opened file. Open the sample: it opens in 360°.
- [ ] Open an ordinary photo: it opens flat.
- [ ] Open each format (PNG, BMP, GIF, TIFF, WebP): all are shown.
- [ ] Open the broken file: an error page with the path instead of a message box. Reload, Close, Show in file manager and Delete work, zoom and projection are disabled.
- [ ] **Win** / **Linux** Start with a directory (`KUK360 <directory>`), then open a file, go to full screen and drop a directory onto the window: an error page, full screen ends, Reload, Close, Delete and the other file actions are disabled.
- [ ] Drag & drop a file from the file manager onto the window: it opens, the source file stays.
- [ ] "Open with" KUK360 from the file manager opens the file. **Mac** also with KUK360 already running. **Linux** only when the AppImage is integrated into the desktop by a tool like Gear Lever.
- [ ] **Win** / **Linux** Start from the command line with a file: `KUK360 <file>`.
- [ ] The window title shows the file path.

## Navigation

- [ ] Left / Right arrow and the panel buttons go through all images in the folder in natural order (`pano2` before `pano10`) and wrap around at the ends.
- [ ] Add a new image to the folder while KUK360 runs, switch to another app and back: Next reaches the new file.
- [ ] In a folder with a single image, Previous and Next are disabled.

## 360° projection

- [ ] Mouse drag looks around with inertia, the wheel zooms, the view stops at the zenith and nadir.
- [ ] W / A / S / D look around, E / Q zoom; in flat they do nothing.
- [ ] Ctrl + Plus / Minus / 0 and View > Zoom zoom, "Zoom: N%" is shown, Ctrl + 0 returns to 100%.
- [ ] L / R start auto-rotation ("Rotation left: N"), repeated presses speed it up, the other direction slows it down and stops it, any drag stops it.
- [ ] Resize the window to taller than wide: the field of view narrows; back to wider: it widens.
- [ ] F4 grid: lines every 10°, red equator, lines stay sharp when looking around.
- [ ] The large panorama opens and has no visible seam at the back (±180°).

## Flat projection

- [ ] F2 or the globe/map button switches the projection, "Flat projection" / "360° projection" is shown and the button icon changes.
- [ ] The image fits the window and stays fitted while resizing.
- [ ] Wheel zoom zooms under the cursor, the limit is 400%, 100% is one image pixel per screen pixel (check with display scaling above 100%). Ctrl + 0 shows "Zoom: 100%".
- [ ] Dragging pans a zoomed image.
- [ ] L / R rotate the image by 90°, the file doesn't change.
- [ ] F4 grid: green/cyan lines only over the image.

## Projection lock

- [ ] Without an open file the lock button is enabled, F3 shows "Projection locked" / "Projection unlocked".
- [ ] F3 with a flat photo shown, then Next to the panorama: it stays flat. The lock button shows a closed lock.
- [ ] F3 again, then Reload (F5): the panorama opens in 360°.

## File actions

- [ ] Ctrl + L opens the file manager with the file selected.
- [ ] Ctrl + C, then paste in the file manager: the file is copied.
- [ ] DEL (**Mac** Cmd + Backspace): the question names the file and the Trash / Recycle Bin, No keeps the file, Yes moves it to the trash and opens the next image. The last image in the folder returns to the start page.
- [ ] Ctrl + W closes the image and shows the start page.
- [ ] The context menu (right click) contains all actions of the main menu except Help and they work.

## Window and full screen

- [ ] F12 and the panel button: only the image is shown, menu and panel hidden. Escape and F12 return the previous state (normal or maximized).
- [ ] In full screen: the context menu and the delete question are shown in front of the image, all shortcuts work.
- [ ] **Mac** The green title button zooms the window, View has no "Enter Full Screen".
- [ ] Move and resize the window, quit, start again: same size and position (**Linux** on Wayland only the same size, the compositor chooses the position). The same with a maximized window. Quit in full screen: the next start is not in full screen.
- [ ] Touch screen or trackpad (if available): pinch zooms in both projections, one finger looks around in 360°.

## Appearance

- [ ] Switch the system between light and dark mode while KUK360 runs: the window, the panel and the menus follow.
- [ ] **Win** In a virtual machine or over remote desktop (no OpenGL driver) the 360° view works through the software renderer.
- [ ] Menu texts are not clipped, the icons are sharp at display scaling 100%, 150% and 200% (**Win**) or on Retina (**Mac**).

## Updates

- [ ] Help > Check for updates with the new version already in `LatestVersionInfo.xml`: "Application is up to date."
- [ ] Set `lastUpdateCheck` in the settings to more than 14 days ago (macOS: `defaults write com.kuk360.KUK360 lastUpdateCheck -string 2026-01-01T00:00:00Z`, not by editing the plist) and start: the reminder question appears after the window; No doesn't ask again on the next start.
- [ ] Set `lastUpdateCheck` to an old time again and start in full screen (F12, quit, start): no question. Set it to a time in the future and start: no question, the value becomes the current time.
- [ ] With the previous version installed: Check for updates offers the new version and opens the downloads page.

## Uninstall

- [ ] **Win** Uninstall from "Installed apps" removes the program folder and the Start menu shortcut, "Open with" no longer offers KUK360.
