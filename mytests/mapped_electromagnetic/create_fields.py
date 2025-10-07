import numpy as np
import pandas as pd

from pathlib import Path

o_dir = Path(__file__).parent

xs_e = np.linspace(-1.5, 1, 21)
xs_b = np.linspace(-1, 1, 257)
ys_e = np.linspace(-1.75, 1, 10)
ys_b = np.linspace(-1, 1, 45)
zs_e = np.linspace(-2, 2, 100)
zs_b = np.linspace(-2, 2, 10)

e_field_strength = 1.0e4  # V/m
b_field_strength = 1.0e1  # G


header_e = ["X [m]", "Y [m]", "Z [m]", "Ex [V/m]", "Ey [V/m]", "Ez [V/m]"]
header_b = ["X [m]", "Y [m]", "Z [m]", "Bx [G]", "By [G]", "Bz [G]"]

data_e = []
for x in xs_e:
    for y in ys_e:
        for z in zs_e:
            data_e.append([x, y, z, 0.0, e_field_strength, 0.0])
df_e = pd.DataFrame(data_e, columns=header_e)
df_e.to_csv(o_dir / "electric_field.csv", index=False)
data_b = []
for x in xs_b:
    for y in ys_b:
        for z in zs_b:
            data_b.append([x, y, z, b_field_strength, 0.0, 0.0])
df_b = pd.DataFrame(data_b, columns=header_b)
df_b.to_csv(o_dir / "magnetic_field.csv", index=False)
