import pandas as pd

# Load your file
df = pd.read_csv("results_log.csv")

# Drop duplicate rows across all columns
df_clean = df.drop_duplicates()

# See how many duplicates were removed
print(f"Removed {len(df) - len(df_clean)} duplicates")

# Save the cleaned data if needed
df_clean.to_csv("results_log_cleaned.csv", index=False)
