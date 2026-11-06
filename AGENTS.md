# Guidelines for AI Coding Agents & Models

Welcome to the **AbioticTrackIR / TrackOpen** repository. This document specifies the standard conventions and commands for AI models working in this codebase.

---

## 🚀 Git & Publishing Conventions

### 1. Default Branch & Remote
* **Default Branch:** `main`
* **Remote:** `origin` (`https://github.com/Fizzolas/AbioticTrackIR.git`)
* **Git Credential Helper:** Windows Credential Manager (`manager`) is globally configured with authenticated HTTPS credentials for user `Fizzolas`.
* **Non-Interactive Execution:** Do not prompt the user for credentials or run interactive authentication wizards. Git over HTTPS pushes seamlessly.
* `push.autoSetupRemote` is enabled globally, so standard `git push` automatically tracks upstream branches without needing `-u origin <branch>`.

### 2. Standard Commit & Push Workflow
When completing a feature, bugfix, or update:
```bash
# 1. Review status
git status

# 2. Stage changes (respecting .gitignore)
git add <files>

# 3. Commit with concise conventional message
git commit -m "feat: description of change"

# 4. Push directly to remote
git push
```

### 3. GitHub Pages (Documentation & Web Showcase)
* The repository's static web showcase is located in [`docs/index.html`](file:///d:/OpenTrack/TrackOpen/docs/index.html).
* An automated GitHub Actions workflow [`.github/workflows/pages.yml`](file:///d:/OpenTrack/TrackOpen/.github/workflows/pages.yml) triggers on every push to `main` to build and deploy to GitHub Pages.
* When updating features, documentation, or releases, update both [`README.md`](file:///d:/OpenTrack/TrackOpen/README.md) and [`docs/index.html`](file:///d:/OpenTrack/TrackOpen/docs/index.html).

---

## 🛠️ Build & Mod Packaging
* **Source:** `AbioticTrackIR/src/` & `AbioticTrackIR/include/`
* **Local Test Scripts:** `INSTALL_LOCAL.bat` copies built binaries to the local Abiotic Factor Steam directory.
* **ModIO Distribution:** `ModIO_Package/` contains the drop-in mod package layout with `AbioticTrackIR.uplugin`.
