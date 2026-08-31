import os
import shutil
import zipfile

dist_dir = 'dist'
if os.path.exists(dist_dir):
    shutil.rmtree(dist_dir)
os.makedirs(dist_dir, exist_ok=True)

release_dir = os.path.abspath('build/Release')
runtime_zip = os.path.abspath('dist/runtime.zip')

print("Packing runtime files into runtime.zip...")
with zipfile.ZipFile(runtime_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
    for root, dirs, files in os.walk(release_dir):
        for file in files:
            if file.lower() in ['acbo_tests.exe', 'acbo_tests.pdb', 'acmodorganize.pdb', 'acmodorganize.exp', 'acmodorganize.lib', 'acbo.exe', 'acmodorganizer.exe']:
                continue
            abs_path = os.path.join(root, file)
            rel_path = os.path.relpath(abs_path, release_dir)
            zf.write(abs_path, rel_path)

print(f"Runtime zip created: {os.path.getsize(runtime_zip) / (1024*1024):.2f} MB")

launcher_bin = 'ACBO.exe'
print(f"Using launcher: {launcher_bin} ({os.path.getsize(launcher_bin)} bytes)")

with open(launcher_bin, 'rb') as f_launcher:
    launcher_data = f_launcher.read()

with open(runtime_zip, 'rb') as f_zip:
    zip_data = f_zip.read()

final_exe = os.path.join(dist_dir, 'ACModOrganizer.exe')
with open(final_exe, 'wb') as f_out:
    f_out.write(launcher_data)
    f_out.write(zip_data)

os.remove(runtime_zip)
print(f"SUCCESS! Single-file standalone executable created: {final_exe} ({os.path.getsize(final_exe) / (1024*1024):.2f} MB)")
