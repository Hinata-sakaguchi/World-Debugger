with open("C:/Users/ryuai/OneDrive/ドキュメント/blender/obj/sea.obj", "r", encoding="utf-8") as f:
    data = f.read()

v = []
vt = []
vn = []
f = []

for i in data.split("\n"):
    if i.startswith("v "):
        v.append(i.split()[1:])  # 三次元座標をそのまま追加
    elif i.startswith("vt "):
        vt.append(i.split()[1:])  # テクスチャ座標をそのまま追加
    elif i.startswith("vn "):
        vn.append(i.split()[1:])  # 法線ベクトルをそのまま追加
    elif i.startswith("f "):
        f.append(i.split()[1:])  # 面情報をそのまま追加

import re

pattern = re.compile(r'\d+/\d+/\d+', re.DOTALL)
matches = pattern.findall(data)

with open("result.txt", "w") as f:
    f.write("")

with open("result.txt", "a") as f:
    for match in matches:
        indices = re.findall(r'\d+/\d+/\d+', match)
        for index in indices:
            v_index, vt_index, vn_index = index.split("/")
            f.write(
                f"\t{{ {{"
                f"{', '.join(v[int(v_index) - 1])}"
                f"}}, {{"
                f"{', '.join(vt[int(vt_index) - 1])}"
                f"}}, {{"
                f"{', '.join(vn[int(vn_index) - 1])}"
                f"}} }},\n"
            )
