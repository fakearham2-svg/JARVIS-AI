# JARVIS AI Android

This folder contains the Android app wrapper for the existing JARVIS AI service.

## What it does

- Installs as **JARVIS AI** on Android (application ID `com.jarvisai.assistant`).
- Opens the existing Railway-hosted JARVIS service inside a Capacitor Android app.
- Keeps the existing NVIDIA chat, projects, search, file upload, and voice features on the same backend.
- Uses HTTPS. An internet connection is required; this first app build is a cloud-connected client, not an offline AI model.

## Build an APK

The GitHub Actions workflow builds a debug-signed APK whenever `mobile/**` changes on `main`, or when manually started from the Actions tab.

1. Open the repository's **Actions** tab and select **Build JARVIS Android APK**.
2. Wait for the build to finish successfully.
3. Download the `JARVIS-AI-Android-APK` artifact, or use the APK attached to the generated prerelease on the Releases page.
4. On Android, open the APK and allow installation from that source if prompted.

The APK is debug-signed for testing and direct installation. It is not a Play Store production-signed release. App-local browser storage is separate from Chrome, so enter your NVIDIA and optional ElevenLabs keys in the app's Settings the first time.

## Local build

Requires Node.js 22+, Java 21, and Android SDK API 36.

```bash
cd mobile
npm install
npx cap add android
npx cap sync android
cd android
./gradlew assembleDebug
```

APK output: `mobile/android/app/build/outputs/apk/debug/app-debug.apk`.
