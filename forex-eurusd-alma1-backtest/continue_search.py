import json
import time
from pathlib import Path

import numpy as np

import backtest_alma as bt


def main():
    df, n, o, h, l, c, atr_prev, indicator, means, raw = bt.load_prices()
    policies = bt.make_policies()
    qv = np.percentile(raw, np.arange(1, 100, dtype=float))
    thresholds = {p: float(qv[p - 1]) for p in range(1, 100)}
    checkpoint = bt.WORK / 'checkpoint.jsonl'
    summaries = [json.loads(line) for line in checkpoint.read_text(encoding='utf-8').splitlines() if line.strip()]
    tested = {(s['cond'], s['lo_pct'], s.get('hi_pct')) for s in summaries}
    candidates = []
    for cond in ('LT', 'GT'):
        candidates.extend((cond, p, None) for p in range(1, 100))
    for lo in range(1, 99):
        candidates.extend(('BETWEEN', lo, hi) for hi in range(lo + 1, 100))
    order = {'LT': 0, 'GT': 1, 'BETWEEN': 2}
    candidates = sorted((c for c in candidates if c not in tested), key=lambda c: (order[c[0]], c[1], c[2] or 0))
    exits, gross = bt.compile_outcomes(n, o, h, l, c, atr_prev, policies)
    print(f'Continuation du maillage 1%: {len(tested)} déjà testés; {len(candidates)} restants; plafond 15 min.', flush=True)
    start = time.monotonic()
    added = 0
    found = None
    for cond, lo, hi in candidates:
        s = bt.evaluate_candidate(cond, lo, hi, thresholds, means, policies, exits, gross, o, atr_prev, n)
        summaries.append(s)
        added += 1
        with open(checkpoint, 'a', encoding='utf-8') as f:
            f.write(json.dumps(s, ensure_ascii=False, allow_nan=True) + '\n')
        if added % 25 == 0:
            print(f'  {len(tested)+added}/{len(tested)+len(candidates)}; {added} nouveaux en {time.monotonic()-start:.0f}s', flush=True)
        if s.get('valid_best'):
            found = s['valid_best']
            print('CANDIDAT VALIDÉ', json.dumps(found, ensure_ascii=False), flush=True)
            break
        if time.monotonic() - start >= 900:
            break
    result_path, reject_path, validated, best_by = bt.save_outputs(summaries, df, policies, n, raw, thresholds)
    state = {
        'n_bars': n,
        'raw_valid_first_half': len(raw),
        'tested_thresholds': len({(s['cond'], s['lo_pct'], s.get('hi_pct')) for s in summaries}),
        'strategies_validated': len(validated),
        'best_by_condition': best_by,
        'results_path': result_path,
        'reject_path': reject_path,
        'continuation_added': added,
        'time_seconds': time.monotonic() - start,
    }
    (bt.WORK / 'progress.json').write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding='utf-8')
    print('TERMINÉ', json.dumps(state, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    main()
