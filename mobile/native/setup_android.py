from pathlib import Path
import re, shutil
root=Path("mobile/android")
pkg=root/"app/src/main/java/com/jarvisai/assistant"
pkg.mkdir(parents=True,exist_ok=True)
shutil.copy("mobile/native/JarvisLocalModelsPlugin.java",pkg/"JarvisLocalModelsPlugin.java")
activity=pkg/"MainActivity.java"
if not activity.exists():
    activity.write_text("package com.jarvisai.assistant;\nimport com.getcapacitor.BridgeActivity;\npublic class MainActivity extends BridgeActivity { @Override public void onCreate(android.os.Bundle savedInstanceState) { registerPlugin(JarvisLocalModelsPlugin.class); super.onCreate(savedInstanceState); } }\n")
else:
    s=activity.read_text()
    if "JarvisLocalModelsPlugin" not in s:
        s=s.replace("public class MainActivity extends BridgeActivity {","public class MainActivity extends BridgeActivity {\n @Override public void onCreate(android.os.Bundle savedInstanceState) { registerPlugin(JarvisLocalModelsPlugin.class); super.onCreate(savedInstanceState); }")
        activity.write_text(s)

gradle=root/"app/build.gradle"
if gradle.exists():
    g=gradle.read_text()
    if "jarvis_llama" not in g:
        g=g.replace("android {", """android {
    defaultConfig { ndk { abiFilters 'arm64-v8a' } }
    externalNativeBuild { cmake { path file('../../native/CMakeLists.txt'); version '3.22.1' } }
""",1)
        gradle.write_text(g)
