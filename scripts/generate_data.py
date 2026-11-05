import numpy as np
import pandas as pd
import os

n_values = [
    100000, 200000, 400000, 600000, 800000, 1000000, 
    1200000, 1400000, 1600000, 1800000, 
    2000000, 4000000, 5000000, 6000000, 7000000, 8000000
]

os.makedirs("../data/non_tries", exist_ok=True)
os.makedirs("../data/tries", exist_ok=True)

print("Début de la génération des fichiers CSV (valeurs uniques)...")

for n in n_values:
    print(f"Génération pour n = {n}...")
    
    # CORRECTION ICI : replace=False garantit qu'il n'y a pas de doublons.
    # On pioche dans un réservoir de 100 millions de nombres pour être sûr d'avoir assez de valeurs.
    tableau_non_trie = np.random.choice(np.arange(0, 100000000), size=n, replace=False)
    
    # Sauvegarde non trié
    df_non_trie = pd.DataFrame(tableau_non_trie)
    df_non_trie.to_csv(f"../data/non_tries/tableau_{n}.csv", index=False, header=False)
    
    # Tri et sauvegarde trié
    tableau_trie = np.sort(tableau_non_trie)
    df_trie = pd.DataFrame(tableau_trie)
    df_trie.to_csv(f"../data/tries/tableau_{n}.csv", index=False, header=False)

print("\n✅ Terminé ! Tous les fichiers .csv sont générés avec des valeurs distinctes.")