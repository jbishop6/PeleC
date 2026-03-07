import re
import pandas as pd

# ---------- file names ----------
input_file  = "probe_data_2d.dat"
output_file = "probe_data_2d.xlsx"

# ---------- read file ----------
with open(input_file, "r") as f:
    lines = f.readlines()

header_line = None
data_lines = []

for line in lines:
    line = line.strip()

    # find header with column labels
    if line.startswith("#") and "Time" in line:
        header_line = line
        continue

    # skip comment lines
    if line.startswith("#") or len(line) == 0:
        continue

    data_lines.append(line)

if header_line is None:
    raise RuntimeError("Header line with column names not found.")

# ---------- extract column names ----------
# pattern like "1:Time", "2:P0_density"
columns = re.findall(r"\d+:[^\s]+", header_line)

# ---------- read numeric data ----------
from io import StringIO
data_string = "\n".join(data_lines)

df = pd.read_csv(
    StringIO(data_string),
    delim_whitespace=True,
    header=None,
    engine="python"
)

# trim columns if Excel added extras
df = df.iloc[:, :len(columns)]

df.columns = columns

# ---------- save to Excel ----------
df.to_excel(output_file, index=False)

print("Converted successfully!")
print(f"Rows: {df.shape[0]}")
print(f"Columns: {df.shape[1]}")
print(f"Output file: {output_file}")
