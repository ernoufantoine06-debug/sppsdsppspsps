# EUR/USD ALMA-1 backtest research

This folder contains the search code and the saved search state for the 2-hour EUR/USD history. The source market data is not included.

## Current status

As of 2026-10-10, 1,546 of the 5,049 possible rules on the one-percentile threshold grid were tested across 224 exit configurations each. No threshold passed the requested validation criteria; 3,503 grid rules remain untested. The results workbook is stored on a private ChatGPT Page. The JSON memory records the best candidates and the assumptions used.

- `historique_rejets_ALMA-1.csv`: one row per tested threshold rule, with its best direction and exit configuration and rejection reason.
- `memoire_recherche.json`: search counts, validation settings, and the best candidates by entry condition.
- `backtest_alma.py`: initial coarse threshold scan and local one-percentile refinement.
- `continue_search.py`: resumes from the existing checkpoint and explores remaining threshold rules for up to 15 minutes per run.
- `outcomes.cpp`: C++17 routine used to precompute OHLC stop/target outcomes.

## Running the search

Install Python packages `numpy`, `pandas`, and `openpyxl`, and a C++17 compiler. Place the two private input files in an `attachments/` directory beside the scripts:

- `ALMA - P ONLY.csv`
- `RÉSULTATS INDICATEURS.xlsx`

Then run `python backtest_alma.py` once to create the initial checkpoint and outputs. Run `python continue_search.py` to resume and append further tested rules. The scripts write their checkpoint and generated files under `work/`. The raw market history is not published in this public repository.
