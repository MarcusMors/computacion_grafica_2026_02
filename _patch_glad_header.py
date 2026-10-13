"""Re-apply the LabCompiler once-guard patch to a freshly generated glad2
header-only gl.h, so the implementation block is emitted at most once per
translation unit even when gl.h is reached through several headers.

Usage:  python _patch_glad_header.py <path-to-gl.h>
"""
import sys

path = sys.argv[1]
with open(path, "r", newline="") as f:
    s = f.read()

if "GLAD_GL_IMPLEMENTATION_ONCE" in s:
    print("already patched:", path)
    sys.exit(0)

old_open = "/* Source */\n#ifdef GLAD_GL_IMPLEMENTATION"
new_open = ("/* Source */\n"
            "#ifdef GLAD_GL_IMPLEMENTATION\n"
            "#ifndef GLAD_GL_IMPLEMENTATION_ONCE\n"
            "#define GLAD_GL_IMPLEMENTATION_ONCE")

old_close = "#endif /* GLAD_GL_IMPLEMENTATION */"
new_close = ("#endif /* GLAD_GL_IMPLEMENTATION_ONCE */\n"
             "#endif /* GLAD_GL_IMPLEMENTATION */")

if old_open not in s or old_close not in s:
    print("ERROR: expected markers not found in", path)
    sys.exit(1)

s = s.replace(old_open, new_open, 1)
s = s.replace(old_close, new_close, 1)

with open(path, "w", newline="") as f:
    f.write(s)

print("patched:", path)
